#include "gpioCommands.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class RecordingGPIO : public tm1637::MGPIO {
public:
    std::vector<std::string> calls;
    int callsUntilFailure = -1;

    void initialize() override { record("initialize"); }
    void deinitialize() override { record("deinitialize"); }
    void setClock(tm1637::PinDigitalState state) override {
        record("clock=" + std::to_string(state));
    }
    void setData(tm1637::PinDigitalState state) override {
        record("data=" + std::to_string(state));
    }
    void delayMicroseconds(int usecs) override {
        record("delay=" + std::to_string(usecs));
    }

private:
    const std::thread::id caller = std::this_thread::get_id();

    void record(const std::string& call) {
        require(caller == std::this_thread::get_id(), "Execution changed threads");
        if (callsUntilFailure == 0) {
            throw std::runtime_error("Injected backend failure");
        }
        if (callsUntilFailure > 0) {
            --callsUntilFailure;
        }
        calls.push_back(call);
    }
};

void testRecordingAndDrain() {
    RecordingGPIO gpio;
    tm1637::GPIOCommandBatch batch;
    auto state = tm1637::PIN_HIGH;
    int usecs = 17;
    batch.initialize();
    batch.setClock(state);
    batch.setData(tm1637::PIN_LOW);
    batch.delayMicroseconds(usecs);
    batch.setClock(tm1637::PIN_LOW);
    batch.setData(state);
    batch.deinitialize();
    state = tm1637::PIN_LOW;
    usecs = 999;

    require(gpio.calls.empty(), "Recording executed GPIO operations");
    batch.drain(gpio);
    const std::vector<std::string> expected = {
        "initialize", "clock=1", "data=0", "delay=17",
        "clock=0", "data=1", "deinitialize"
    };
    require(gpio.calls == expected, "Batch changed command order or values");
    batch.drain(gpio);
    require(gpio.calls == expected, "Draining replayed commands");

    batch.delayMicroseconds(0);
    batch.drain(gpio);
    require(gpio.calls.size() == expected.size() + 1 && gpio.calls.back() == "delay=0",
            "A drained batch cannot be reused");
}

void testDiscardAndIndependentBatches() {
    RecordingGPIO gpio;
    {
        tm1637::GPIOCommandBatch abandoned;
        abandoned.setClock(tm1637::PIN_HIGH);
    }
    require(gpio.calls.empty(), "Discarded recording executed commands");

    tm1637::GPIOCommandBatch first;
    tm1637::GPIOCommandBatch second;
    first.setClock(tm1637::PIN_HIGH);
    second.setData(tm1637::PIN_LOW);
    first.drain(gpio);
    require(gpio.calls == std::vector<std::string>{"clock=1"},
            "Draining one batch executed another");
    second.drain(gpio);
    require(gpio.calls == std::vector<std::string>{"clock=1", "data=0"},
            "Independent batch was lost");
}

void testFailedDrain(int callsBeforeFailure) {
    RecordingGPIO gpio;
    tm1637::GPIOCommandBatch batch;
    batch.initialize();
    batch.setClock(tm1637::PIN_LOW);
    batch.setData(tm1637::PIN_LOW);
    gpio.callsUntilFailure = callsBeforeFailure;
    bool failed = false;
    try {
        batch.drain(gpio);
    } catch (const std::runtime_error&) {
        failed = true;
    }
    require(failed, "Backend exception was swallowed");
    require(gpio.calls.size() == static_cast<std::size_t>(callsBeforeFailure),
            "Execution continued after a backend failure");
    gpio.callsUntilFailure = -1;
    batch.drain(gpio);
    require(gpio.calls.size() == static_cast<std::size_t>(callsBeforeFailure),
            "Failed commands were replayed on a later drain");

    // Cleanup can still be explicitly performed without replaying failed work.
    const tm1637::GPIOCommand shutdown{tm1637::GPIOCommand::Kind::Deinitialize, 0};
    shutdown.execute(gpio);
    require(gpio.calls.back() == "deinitialize", "Shutdown command did not execute");
}
}

int main() {
    try {
        testRecordingAndDrain();
        testDiscardAndIndependentBatches();
        testFailedDrain(0); // Initialization fails before any pin operations.
        testFailedDrain(1);
        testFailedDrain(2);
        std::cout << "Command recording, ordering, lifecycle, and failure checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
