#include "fakeWiringPi.h"

#include <stdexcept>
#include <mutex>

namespace {
std::vector<fake_wiringpi::Call> recordedCalls;
int writesUntilFailure = -1;
std::mutex callsMutex;

void recordUnlocked(fake_wiringpi::Kind kind, int first = 0, int second = 0) {
    recordedCalls.push_back({kind, first, second, std::this_thread::get_id()});
}

void record(fake_wiringpi::Kind kind, int first = 0, int second = 0) {
    std::lock_guard<std::mutex> lock(callsMutex);
    recordUnlocked(kind, first, second);
}
}

namespace fake_wiringpi {
std::vector<Call> calls() {
    std::lock_guard<std::mutex> lock(callsMutex);
    return recordedCalls;
}

void reset() {
    std::lock_guard<std::mutex> lock(callsMutex);
    recordedCalls.clear();
    writesUntilFailure = -1;
}

void failAfterWrites(int count) {
    std::lock_guard<std::mutex> lock(callsMutex);
    writesUntilFailure = count;
}
}

extern "C" {
int wiringPiSetup() {
    record(fake_wiringpi::Kind::Setup);
    return 0;
}

int wiringPiSetupGpio() {
    record(fake_wiringpi::Kind::SetupBCM);
    return 0;
}

void pinMode(int pin, int mode) {
    record(fake_wiringpi::Kind::PinMode, pin, mode);
}

void digitalWrite(int pin, int value) {
    std::lock_guard<std::mutex> lock(callsMutex);
    if (writesUntilFailure == 0) {
        throw std::runtime_error("Injected GPIO write failure");
    }
    if (writesUntilFailure > 0) {
        --writesUntilFailure;
    }
    recordUnlocked(fake_wiringpi::Kind::Write, pin, value);
}

void delayMicroseconds(unsigned int usecs) {
    record(fake_wiringpi::Kind::Delay, static_cast<int>(usecs));
}
}
