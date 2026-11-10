#include "gpioExecutor.h"
#include "gpioGPIODv1.h"
#include "gpioPigpioBase.h"

#include <cmath>
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>

extern "C" {
void fakeGpiodFailure(const char* operation);
int fakeGpiodResources();
int fakeGpiodInvalidReleases();
}

namespace {
using namespace tm1637;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void checkReleased() {
    require(fakeGpiodResources() == 0, "GPIOD resource leak");
    require(fakeGpiodInvalidReleases() == 0, "Invalid GPIOD release");
}

void testGpiodV2() {
    for (const auto mode : {ExecutionMode::Inline, ExecutionMode::Worker}) {
        for (const auto stage : {"open", "settings", "direction", "output", "config",
                                 "add", "request-config", "request", "write"}) {
            fakeGpiodFailure(stage);
            bool failed = false;
            try { Device device(23, 24, GpioGPIOD, DeviceOptions{mode}); }
            catch (const std::runtime_error&) { failed = true; }
            require(failed, "GPIOD v2 initialization error was ignored");
            checkReleased();
        }
        fakeGpiodFailure("");
        {
            Device device(23, 24, GpioGPIOD, DeviceOptions{mode});
            fakeGpiodFailure("write");
            bool failed = false;
            try { device.clear(); device.flush(); }
            catch (const std::runtime_error&) { failed = true; }
            require(failed, "GPIOD v2 write failure was ignored");
        }
        checkReleased();
    }
    fakeGpiodFailure("");
    { Device device(23, 24); device.clear(); }
    checkReleased();
}

void testGpiodV1() {
    void* handle = dlopen("libgpiod.so.3", RTLD_NOW);
    require(handle != nullptr, "Cannot load recording GPIOD backend");
    for (const auto mode : {ExecutionMode::Inline, ExecutionMode::Worker}) {
        for (const auto stage : {"open", "line", "v1-clock", "v1-data", "write"}) {
            fakeGpiodFailure(stage);
            std::unique_ptr<MGPIO> gpio;
            bool failed = false;
            try {
                GPIOExecutor executor(gpio, [handle] {
                    return std::make_unique<GPIODv1>(23, 24, handle);
                }, mode);
            } catch (const std::runtime_error&) { failed = true; }
            require(failed && !gpio, "GPIOD v1 initialization error was ignored");
            checkReleased();
        }
        fakeGpiodFailure("");
        {
            std::unique_ptr<MGPIO> gpio;
            GPIOExecutor executor(gpio, [handle] {
                return std::make_unique<GPIODv1>(23, 24, handle);
            }, mode);
            fakeGpiodFailure("write");
            GPIOCommandBatch commands;
            commands.setData(PIN_HIGH);
            bool failed = false;
            try { executor.submit(std::move(commands)); executor.flush(); }
            catch (const std::runtime_error&) { failed = true; }
            require(failed, "GPIOD v1 write failure was ignored");
        }
        checkReleased();
    }
    dlclose(handle);
}

double lastDelay = 0;
void captureDelay(double seconds) { lastDelay = seconds; }

class TestPigpio : public PigpioBase {
public:
    TestPigpio() : PigpioBase(23, 24) {}
    bool failPinMode = false;
    int releases = 0;
private:
    void dynLoadLib() override { m_time_sleep = captureDelay; }
    void initializeGpio() override {}
    void deinitializeGpio() override { ++releases; }
    void setPinMode(unsigned, unsigned) override {
        if (failPinMode) { throw std::runtime_error("pin mode"); }
    }
    void gpioWrite(unsigned, PinDigitalState) override {}
};

void testPigpio() {
    { TestPigpio uninitialized; } // Destruction before dlopen must be safe.
    TestPigpio gpio;
    gpio.initialize();
    gpio.delayMicroseconds(10);
    require(std::abs(lastDelay - 0.00001) < 1e-12, "Pigpio delay lost fractional seconds");
    gpio.deinitialize();
    gpio.deinitialize();
    require(gpio.releases == 1, "Pigpio teardown repeated");
    gpio.failPinMode = true;
    bool failed = false;
    try { gpio.initialize(); } catch (const std::runtime_error&) { failed = true; }
    gpio.deinitialize();
    require(failed && gpio.releases == 2, "Partial Pigpio startup did not release GPIO");
}
}

int main() {
    try {
        testGpiodV2();
        testGpiodV1();
        testPigpio();
        std::cout << "Backend partial initialization, GPIO errors, and cleanup checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
