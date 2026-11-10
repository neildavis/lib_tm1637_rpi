#pragma once

#include "gpioCommands.h"
#include "tm1637.h"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>

namespace tm1637 {

// Manages the existing Device backend storage. All backend access, including
// creation and destruction, belongs to the selected execution thread.
class GPIOExecutor {
public:
    using Factory = std::function<std::unique_ptr<MGPIO>()>;

    GPIOExecutor(std::unique_ptr<MGPIO>& gpio, Factory factory, ExecutionMode mode);
    ~GPIOExecutor();
    GPIOExecutor(const GPIOExecutor&) = delete;
    GPIOExecutor& operator=(const GPIOExecutor&) = delete;

    void submit(GPIOCommandBatch&& commands);
    void wait(std::chrono::milliseconds duration);
    void flush();

private:
    struct Request {
        std::uint64_t sequence;
        GPIOCommandBatch commands;
        std::chrono::milliseconds pause;
    };

    void enqueue(GPIOCommandBatch&& commands, std::chrono::milliseconds pause);
    void initialize();
    void cleanup() noexcept;
    void run() noexcept;
    void fail(std::exception_ptr error);
    void checkFailure() const;

    std::unique_ptr<MGPIO>& m_gpio;
    Factory m_factory;
    ExecutionMode m_mode;
    std::mutex m_mutex;
    std::condition_variable m_workAvailable;
    std::condition_variable m_stateChanged;
    std::deque<Request> m_queue;
    std::uint64_t m_submitted = 0;
    std::uint64_t m_completed = 0;
    bool m_ready = false;
    bool m_stopping = false;
    std::exception_ptr m_failure;
    std::thread m_thread;
};

}
