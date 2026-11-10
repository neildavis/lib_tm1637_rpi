#pragma once

#include <thread>
#include <vector>

namespace fake_wiringpi {

enum class Kind { Setup, SetupBCM, PinMode, Write, Delay };

struct Call {
    Kind kind;
    int first;
    int second;
    std::thread::id thread;
};

const std::vector<Call>& calls();
void reset();
void failAfterWrites(int count);

}
