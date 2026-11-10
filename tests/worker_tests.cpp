#include "gpioExecutor.h"

#include <cstdlib>
#include <future>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

// Inject allocation failure only on the producer, without affecting the worker.
thread_local bool rejectAllocations = false;
void* operator new(std::size_t size) {
    if (rejectAllocations) { throw std::bad_alloc(); }
    if (void* memory = std::malloc(size ? size : 1)) { return memory; }
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* memory) noexcept { ::operator delete(memory); }
void operator delete[](void* memory, std::size_t) noexcept { ::operator delete(memory); }

namespace {
using namespace tm1637;
using namespace std::chrono;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

struct Event {
    std::string name;
    std::thread::id thread;
    steady_clock::time_point time;
};

struct State {
    std::mutex mutex;
    std::condition_variable changed;
    std::vector<Event> events;
    std::string gate;
    std::string failure;
    bool blocked = false;
    bool released = false;
    bool failCleanup = false;

    void record(const std::string& name) {
        std::unique_lock<std::mutex> lock(mutex);
        events.push_back({name, std::this_thread::get_id(), steady_clock::now()});
        if (gate == name && !released) {
            blocked = true;
            changed.notify_all();
            require(changed.wait_for(lock, seconds(3), [this] { return released; }),
                    "Test gate was not released");
        }
        if (failure == name || (failCleanup && name == "deinitialize")) {
            throw std::runtime_error(name);
        }
    }

    void awaitBlocked() {
        std::unique_lock<std::mutex> lock(mutex);
        require(changed.wait_for(lock, seconds(3), [this] { return blocked; }),
                "Worker did not reach test gate");
    }

    void release() {
        std::lock_guard<std::mutex> lock(mutex);
        released = true;
        changed.notify_all();
    }

    std::vector<Event> snapshot() {
        std::lock_guard<std::mutex> lock(mutex);
        return events;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        events.clear();
    }
};

class Backend : public MGPIO {
public:
    explicit Backend(State& state) : m_state(state) { m_state.record("construct"); }
    ~Backend() override { m_state.record("destroy"); }
    void initialize() override { m_state.record("initialize"); }
    void deinitialize() override { m_state.record("deinitialize"); }
    void setClock(PinDigitalState value) override {
        m_state.record("clock=" + std::to_string(value));
    }
    void setData(PinDigitalState value) override {
        m_state.record("data=" + std::to_string(value));
    }
    void delayMicroseconds(int usecs) override {
        m_state.record("delay=" + std::to_string(usecs));
    }
private:
    State& m_state;
};

GPIOExecutor::Factory factory(State& state) {
    return [&state] { return std::make_unique<Backend>(state); };
}

GPIOCommandBatch data(PinDigitalState value) {
    GPIOCommandBatch commands(1);
    commands.setData(value);
    return commands;
}

void checkThreads(const std::vector<Event>& events, bool worker) {
    require(!events.empty(), "No backend lifecycle events");
    const auto thread = events.front().thread;
    require((thread != std::this_thread::get_id()) == worker, "Wrong execution thread");
    for (const auto& event : events) {
        require(event.thread == thread, "Backend lifecycle crossed threads");
    }
}

void testInline() {
    State state;
    std::unique_ptr<MGPIO> gpio;
    {
        GPIOExecutor executor(gpio, factory(state), ExecutionMode::Inline);
        executor.submit(data(PIN_HIGH));
        require(state.snapshot().back().name == "data=1", "Inline submit returned early");
        const auto start = steady_clock::now();
        executor.wait(milliseconds(20));
        require(steady_clock::now() - start >= milliseconds(20), "Inline wait returned early");
        executor.wait(milliseconds(-1));
        executor.wait(milliseconds(0));
        executor.flush();
    }
    require(!gpio, "Inline backend was not destroyed");
    checkThreads(state.snapshot(), false);
}

void testStartupHandshake() {
    State state;
    state.gate = "initialize";
    std::unique_ptr<MGPIO> gpio;
    auto constructing = std::async(std::launch::async, [&] {
        return std::make_unique<GPIOExecutor>(gpio, factory(state), ExecutionMode::Worker);
    });
    state.awaitBlocked();
    const bool waiting = constructing.wait_for(milliseconds(30)) == std::future_status::timeout;
    state.release();
    auto executor = constructing.get();
    require(waiting, "Constructor returned before initialization completed");
    const auto events = state.snapshot();
    require(events.back().name == "data=0", "Constructor omitted initial pin states");
    executor.reset();
    checkThreads(state.snapshot(), true);
}

void testStartupFailures() {
    for (const auto mode : {ExecutionMode::Inline, ExecutionMode::Worker}) {
        for (const std::string stage : {"initialize", "clock=0", "data=0"}) {
            State state;
            state.failure = stage;
            std::unique_ptr<MGPIO> gpio;
            bool failed = false;
            try { GPIOExecutor executor(gpio, factory(state), mode); }
            catch (const std::runtime_error& error) { failed = error.what() == stage; }
            require(failed && !gpio, "Startup failure did not clean up and propagate");
            const auto events = state.snapshot();
            require(events[events.size() - 2].name == "deinitialize" &&
                    events.back().name == "destroy", "Startup cleanup incomplete");
            checkThreads(events, mode == ExecutionMode::Worker);
        }
        std::unique_ptr<MGPIO> gpio;
        bool failed = false;
        try {
            GPIOExecutor executor(gpio, []() -> std::unique_ptr<MGPIO> {
                throw std::runtime_error("factory");
            }, mode);
        } catch (const std::runtime_error& error) { failed = std::string(error.what()) == "factory"; }
        require(failed && !gpio, "Factory failure was lost");
    }
}

void testFIFOAndInFlightFlush() {
    State state;
    state.gate = "data=1";
    std::unique_ptr<MGPIO> gpio;
    GPIOExecutor executor(gpio, factory(state), ExecutionMode::Worker);
    state.clear();
    executor.submit(data(PIN_HIGH));
    state.awaitBlocked(); // The queue is empty, but a batch is still executing.
    auto flushing = std::async(std::launch::async, [&] { executor.flush(); });
    const bool waiting = flushing.wait_for(milliseconds(30)) == std::future_status::timeout;

    executor.wait(milliseconds(25));
    // More than 64 entries must be accepted while the worker remains blocked.
    for (int i = 0; i < 130; ++i) {
        executor.submit(data(i % 2 ? PIN_HIGH : PIN_LOW));
    }
    require(state.snapshot().size() == 1, "Submission executed or preempted pending work");
    const auto releasedAt = steady_clock::now();
    state.release();
    flushing.get();
    require(waiting, "Flush ignored the executing batch");
    executor.flush();
    const auto events = state.snapshot();
    require(events.size() == 131, "Queued updates were lost or replayed");
    require(events[1].time - releasedAt >= milliseconds(25), "Queued pause was skipped");
    for (int i = 0; i < 130; ++i) {
        require(events[i + 1].name == (i % 2 ? "data=1" : "data=0"), "FIFO order changed");
    }
    checkThreads(events, true);
}

void testAllocationFailure() {
    State state;
    state.gate = "data=1";
    std::unique_ptr<MGPIO> gpio;
    GPIOExecutor executor(gpio, factory(state), ExecutionMode::Worker);
    state.clear();
    executor.submit(data(PIN_HIGH));
    state.awaitBlocked();
    int accepted = 0;
    bool failed = false;
    for (int i = 0; i < 1024 && !failed; ++i) {
        auto commands = data(PIN_LOW);
        rejectAllocations = true;
        try { executor.submit(std::move(commands)); }
        catch (const std::bad_alloc&) { failed = true; }
        rejectAllocations = false;
        if (!failed) { ++accepted; }
    }
    executor.submit(data(PIN_LOW)); // Allocation failure must not poison the worker.
    state.release();
    executor.flush();
    require(failed, "Allocation failure was not exercised");
    require(state.snapshot().size() == static_cast<std::size_t>(accepted + 2),
            "Allocation failure lost accepted work or published rejected work");
}

void testFailureAndCleanup() {
    State state;
    state.gate = "data=1";
    state.failure = "data=1";
    state.failCleanup = true;
    std::unique_ptr<MGPIO> gpio;
    {
        GPIOExecutor executor(gpio, factory(state), ExecutionMode::Worker);
        state.clear();
        executor.submit(data(PIN_HIGH));
        state.awaitBlocked();
        executor.wait(milliseconds(100));
        executor.submit(data(PIN_LOW));
        auto flushing = std::async(std::launch::async, [&] {
            try { executor.flush(); } catch (const std::runtime_error& error) {
                return std::string(error.what()) == "data=1";
            }
            return false;
        });
        state.release();
        require(flushing.get(), "Flush did not report the original worker error");
        bool failed = false;
        try { executor.submit(data(PIN_LOW)); }
        catch (const std::runtime_error& error) { failed = std::string(error.what()) == "data=1"; }
        require(failed, "Failed worker accepted work");
        failed = false;
        try { executor.wait(milliseconds(0)); }
        catch (const std::runtime_error&) { failed = true; }
        require(failed, "Wait did not report worker failure");
    }
    const auto events = state.snapshot();
    require(!gpio && events.size() == 3 && events[0].name == "data=1" &&
            events[1].name == "deinitialize" && events[2].name == "destroy",
            "Failure did not discard pending work and finish cleanup");
    checkThreads(events, true);
}

void testDestructionDrains() {
    State state;
    std::unique_ptr<MGPIO> gpio;
    {
        GPIOExecutor executor(gpio, factory(state), ExecutionMode::Worker);
        state.clear();
        executor.submit(data(PIN_HIGH));
        executor.wait(milliseconds(20));
        executor.submit(data(PIN_LOW));
        // No flush: destruction must finish updates and pauses before cleanup.
    }
    const auto events = state.snapshot();
    require(!gpio && events.size() == 4 && events[0].name == "data=1" &&
            events[1].name == "data=0" && events[2].name == "deinitialize" &&
            events[3].name == "destroy", "Destruction did not drain in order");
    require(events[1].time - events[0].time >= milliseconds(20), "Destruction skipped a pause");
    checkThreads(events, true);
}
}

int main() {
    try {
        testInline();
        testStartupHandshake();
        testStartupFailures();
        testFIFOAndInFlightFlush();
        testAllocationFailure();
        testFailureAndCleanup();
        testDestructionDrains();
        std::cout << "Worker lifecycle, FIFO, waits, completion, and failure tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        rejectAllocations = false;
        std::cerr << error.what() << '\n';
        return 1;
    }
}
