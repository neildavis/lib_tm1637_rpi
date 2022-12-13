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

    /** SegmentMask can be used with bitwise OR to construct raw digits*/
    typedef enum {
        SegNone     = 0b00000000,
        SegA        = 0b00000001,
        SegB        = 0b00000010,
        SegC        = 0b00000100,
        SegD        = 0b00001000,
        SegE        = 0b00010000,
        SegF        = 0b00100000,
        SegG        = 0b01000000,
        SegColon    = 0b10000000
    } SegmentMask;
   
    /** GPIOLib determines the library used for GPIO access */
    typedef enum {
        /**  Use GPIOD (native linux character device) via libgpiod */
        GpioGPIOD,
        /** Use WiringPi with 'WiringPi' pin numbering */  
        GpioWiringPi,
        /** Use WiringPi with 'Broadcom (BCM)' pin numbering */
        GpioWiringPiBCM,
        /** Use pigpio daemon (pigpiod) via libpigpiod_if2 */
        GpioPigpioDaemon,
    } GPIOLib;

    /** Radix determines the radix when displaying integer literals */
    typedef enum {
        RadixDecimal = 10,
        RadixHex = 0x10
    } Radix;

    class MGPIO;
    /** Device is the main type that provides access to the TM1637 */
    class Device {
    public:
        Device(int pinClk, int pinData);
        Device(int pinClk, int pinData, GPIOLib gpioLib);
        virtual ~Device();
        Device(const Device &device) = delete;   // copying is not supported
        Device& operator=(const Device &device) = delete;   // assignment is not supported

        // API
        void displayOff();
        void setBrightnessPercent(int brightnessPercent);
        void clear();
        void setColon(bool showColon);
        void showRawDigits(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);
        void showRawDigits(const uint8_t *digits);
        void showRawDigit(int pos, uint8_t digit);
        void showIntegers(int int1, int int2, int int3, int int4);
        void showIntegers(const int *integers);
        void showInteger(int pos, int integer);
        void showIntegerLiteral(int intLit, Radix radix = RadixDecimal);

    protected:
        // Protocol
        void start() const;
        void stop() const;
        void br() const;
        void writeByte(uint8_t data) const;
        void showCurrentData() const;
        void showDigitAtPos(int pos);

        
    protected:
        MGPIO *m_gpio;
        uint8_t m_data[4];
        uint8_t m_brightness;
    };
}