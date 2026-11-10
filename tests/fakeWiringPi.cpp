#include "fakeWiringPi.h"

#include <stdexcept>

namespace {
std::vector<fake_wiringpi::Call> recordedCalls;
int writesUntilFailure = -1;

void record(fake_wiringpi::Kind kind, int first = 0, int second = 0) {
    recordedCalls.push_back({kind, first, second, std::this_thread::get_id()});
}
}

namespace fake_wiringpi {
const std::vector<Call>& calls() { return recordedCalls; }

void reset() {
    recordedCalls.clear();
    writesUntilFailure = -1;
}

void failAfterWrites(int count) { writesUntilFailure = count; }
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

int pinMode(int pin, int mode) {
    record(fake_wiringpi::Kind::PinMode, pin, mode);
    return 0;
}

void digitalWrite(int pin, int value) {
    if (writesUntilFailure == 0) {
        throw std::runtime_error("Injected GPIO write failure");
    }
    if (writesUntilFailure > 0) {
        --writesUntilFailure;
    }
    record(fake_wiringpi::Kind::Write, pin, value);
}

void delayMicroseconds(unsigned int usecs) {
    record(fake_wiringpi::Kind::Delay, static_cast<int>(usecs));
}
}
