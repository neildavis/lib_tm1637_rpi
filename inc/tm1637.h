#pragma once
#include <inttypes.h>

namespace tm1637 {

    /*
        A
        ---
    F |   | B
        -G-
    E |   | C
        ---
        D

    When setting raw digits:
    seg A is bit 0 (lsb), seg A is bit 1, ..., seg G is bit 7
    colon is bit 8 (msb) - but better to follow up with a call to setColon()
    */

    class MGPIO;

    /** GPIOLib determines the library used for GPIO access */
    typedef enum {
        GpioWiringPi,    // Use WiringPi with 'WiringPi' pin numbering
        GpioWiringPiBCM  // Use WiringPi with 'BCM' pin numbering
    } GPIOLib;

    /** Radix determines the radix when displaying integer literals */
    typedef enum {
        RadixDecimal = 10,
        RadixHex = 0x10
    } Radix;

    class Device {
    public:
        Device(int pinClk, int pinData, GPIOLib gpioLib = GpioWiringPiBCM);
        virtual ~Device();
        Device(const Device &device) = delete;   // copying is not supported
        Device& operator=(const Device &device) = delete;   // assignment is not supported

        // API
        void displayOff();
        void setBrightnessPercent(int brightnessPercent);
        void clear();
        void setColon(bool showColon);
        void showRawDigits(const uint8_t *digits);
        void showRawDigit(int pos, uint8_t digit);
        void showIntegers(const int *integers);
        void showInteger(int pos, int integer);
        void showIntegerLiteral(int intLit, Radix radix);

    protected:
        // Protocol
        void start() const;
        void stop() const;
        inline void br() const { stop(); start(); }
        void writeByte(uint8_t data) const;
        void showCurrentData() const;
        void showDigitAtPos(int pos);

        
    protected:
        MGPIO *m_gpio;
        uint8_t m_data[4];
        uint8_t m_brightness;
    };
}