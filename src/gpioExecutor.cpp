#include "gpioExecutor.h"

#include <stdexcept>
#include <utility>

using namespace tm1637;

GPIOExecutor::GPIOExecutor(std::unique_ptr<MGPIO>& gpio, Factory factory, ExecutionMode mode)
    : m_gpio(gpio), m_factory(std::move(factory)), m_mode(mode) {
    if (mode != ExecutionMode::Inline && mode != ExecutionMode::Worker) {
        throw std::invalid_argument("Invalid TM1637 execution mode");
    }
    if (mode == ExecutionMode::Inline) {
        try {
            initialize();
        } catch (...) {
            cleanup();
            throw;
        }
        return;
    }

    m_thread = std::thread(&GPIOExecutor::run, this);
    std::unique_lock<std::mutex> lock(m_mutex);
    m_stateChanged.wait(lock, [this] { return m_ready; });
    if (m_failure) {
        auto error = m_failure;
        lock.unlock();
        m_thread.join();
        std::rethrow_exception(error);
    }
}

GPIOExecutor::~GPIOExecutor() {
    if (m_mode == ExecutionMode::Worker) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopping = true;
        }
        m_workAvailable.notify_one();
        m_thread.join();
    } else {
        cleanup();
    }
}

void GPIOExecutor::initialize() {
    GPIOCommandBatch startup(3);
    startup.initialize();
    startup.setClock(PIN_LOW);
    startup.setData(PIN_LOW);
    m_gpio = m_factory();
    if (!m_gpio) {
        throw std::runtime_error("GPIO backend factory returned null");
    }
    startup.drain(*m_gpio);
}

void GPIOExecutor::cleanup() noexcept {
    if (m_gpio) {
        try {
            const GPIOCommand shutdown{GPIOCommand::Kind::Deinitialize, 0};
            shutdown.execute(*m_gpio);
        } catch (...) {
            // Destruction cannot report errors. Preserve the first failure for
            // any waiters already observing this executor.
            fail(std::current_exception());
        }
        m_gpio.reset();
    }
}

void GPIOExecutor::checkFailure() const {
    // Caller holds m_mutex.
    if (m_failure) {
        std::rethrow_exception(m_failure);
    }
    if (m_stopping) {
        throw std::runtime_error("TM1637 executor is stopping");
    }
}

void GPIOExecutor::enqueue(GPIOCommandBatch&& commands, std::chrono::milliseconds pause) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        checkFailure();
        // Publish the sequence number only after insertion succeeds. Allocation
        // failure rejects this request without disturbing accepted work.
        const auto sequence = m_submitted + 1;
        m_queue.push_back({sequence, std::move(commands), pause});
        m_submitted = sequence;
    }
    m_workAvailable.notify_one();
}

void GPIOExecutor::submit(GPIOCommandBatch&& commands) {
    if (m_mode == ExecutionMode::Inline) {
        commands.drain(*m_gpio);
    } else {
        enqueue(std::move(commands), std::chrono::milliseconds::zero());
    }
}

void GPIOExecutor::wait(std::chrono::milliseconds duration) {
    if (m_mode == ExecutionMode::Inline) {
        if (duration.count() > 0) {
            std::this_thread::sleep_for(duration);
        }
    } else if (duration.count() > 0) {
        enqueue(GPIOCommandBatch{}, duration);
    } else {
        std::lock_guard<std::mutex> lock(m_mutex);
        checkFailure();
    }
}

void GPIOExecutor::flush() {
    if (m_mode == ExecutionMode::Inline) {
        return;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    const auto target = m_submitted;
    m_stateChanged.wait(lock, [this, target] {
        return m_completed >= target || m_failure;
    });
    if (m_failure) {
        std::rethrow_exception(m_failure);
    }
}

void GPIOExecutor::fail(std::exception_ptr error) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_failure) {
            m_failure = error;
        }
        m_queue.clear();
        m_ready = true;
    }
    m_stateChanged.notify_all();
}

void GPIOExecutor::run() noexcept {
    try {
        initialize();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ready = true;
        }
        m_stateChanged.notify_all();
        for (;;) {
            Request request{0, GPIOCommandBatch{}, std::chrono::milliseconds::zero()};
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_workAvailable.wait(lock, [this] {
                    return m_stopping || !m_queue.empty();
                });
                if (m_queue.empty()) {
                    break; // Stop requested and all accepted work completed.
                }
                request = std::move(m_queue.front());
                m_queue.pop_front();
            }
            request.commands.drain(*m_gpio);
            if (request.pause.count() > 0) {
                std::this_thread::sleep_for(request.pause);
            }
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_completed = request.sequence;
            }
            m_stateChanged.notify_all();
        }
    } catch (...) {
        fail(std::current_exception());
    }
    cleanup();
}
