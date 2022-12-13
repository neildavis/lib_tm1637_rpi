#include <tm1637.h>

#include <chrono>
#include <thread>
#include <iostream>

using namespace tm1637;

#define max(a, b) ((a) < (b) ? (b) : (a))

const uint8_t DIGIT_A = 0b1110111;
const uint8_t DIGIT_C = 0b0111001;
const uint8_t DIGIT_H = 0b1110110;
const uint8_t DIGIT_I = 0b0110000;
const uint8_t DIGIT_L = 0b0111000;
const uint8_t DIGIT_O = 0b0111111;

void raiseDisplay(Device &tm1637) {
    tm1637.displayOff();
    for (int i = 0; i < 106 / 7; i += 15) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        tm1637.setBrightnessPercent(i);
    }
}

void fadeDisplay(Device &tm1637) {
    for (int i = 100; i > -6; i -= 15) {
        tm1637.setBrightnessPercent(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    tm1637.displayOff();
}

// If no GPIO lib is declared on command line, use GPIOD by default
#if !defined(GPIO_LIB)
#define GPIO_LIB GpioGPIOD
#endif

int main() {
    std::cout << "Using GPIO lib " << GPIO_LIB << std::endl;
    Device tm1637(3, 2, GPIO_LIB);
    std::cout << "Press CTRL+C to exit" << std::endl;
    tm1637.clear();

    // Say 'Hello' in a few languages
    tm1637.showRawDigits(0, DIGIT_H, DIGIT_I, 0);
    fadeDisplay(tm1637);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    tm1637.showRawDigits(DIGIT_H, DIGIT_O, DIGIT_L, DIGIT_A);
    fadeDisplay(tm1637);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    tm1637.showRawDigits(DIGIT_C, DIGIT_I, DIGIT_A, DIGIT_O);
    fadeDisplay(tm1637);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    uint8_t halo[] = {DIGIT_H, DIGIT_A, DIGIT_L, DIGIT_O};
    tm1637.showRawDigits(halo);
    fadeDisplay(tm1637);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Segment animation
    tm1637.clear();
    tm1637.setBrightnessPercent(100);
    for (int i = 0; i < 4; i++) {
        for (int seg = 0; seg < 6; seg++) {
            for (int dig = 0; dig < 4; dig++) {
                uint8_t digit = 1 << seg;
                tm1637.showRawDigit(dig, digit);
            }  
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Blink the colon with PWM
    tm1637.clear();
    tm1637.setColon(true);
    for (int dig = 0; dig < 4; dig++) {
        uint8_t digit = SegG | SegColon;
        tm1637.showRawDigit(dig, digit);
    }  
    for (int i = 0; i < 4; i++) {
        raiseDisplay(tm1637);
        fadeDisplay(tm1637);
    }

    // Numeric literal
    tm1637.clear();
    tm1637.setBrightnessPercent(100);
    for (int i = 0; i < 4; i++) {
        tm1637.showIntegerLiteral(1637);
        tm1637.setColon(i % 2 == 0);
        fadeDisplay(tm1637);
        raiseDisplay(tm1637);
        tm1637.displayOff();
    }

    // Integers
    tm1637.clear();
    tm1637.setBrightnessPercent(100);
    for (int i = 0; i < 0x10; i++) {
        tm1637.showIntegers(max(0,i-3), max(0, i-2), max(0, i-1), i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Show an insult using hex words!
    tm1637.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    tm1637.showIntegerLiteral(0xbeef, RadixHex);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    tm1637.showIntegerLiteral(0xface, RadixHex);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Finally loop for ages showing all decimal numbers
    tm1637.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (int i = 0; i < 10000; i++) {
        tm1637.showIntegerLiteral(i, RadixDecimal);
        tm1637.setColon((i / 100) % 2 == 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    tm1637.clear();
    return 0;
}   