#pragma once

#include "gpio.h"
#include "gpioPigpioBase.h"

namespace tm1637 {
    
    class PigpioDaemon : public PigpioBase {
    public:
        PigpioDaemon(int pinClk, int pinData);
    private:
        // Internal template method overrides
        virtual void dynLoadLib() override;
        virtual void initializeGpio() override;
        virtual void deinitializeGpio() override;
        virtual void setPinMode(unsigned pinNum, unsigned mode) override;
        virtual void gpioWrite(unsigned pinNum, PinDigitalState state) override;
    private:
        // These are func pointers we assign from the symbols in libpigpiod_if2.so.1
        int (*m_pigpio_start)(const char *addrStr, const char *addrPortStr);
        void (*m_pigpio_stop)(int pi);
        int (*m_set_mode)(int pi, unsigned gpio, unsigned mode);
        int (*m_gpio_write)(int pi, unsigned gpio, unsigned level);
    private:
        int m_pi;
   };
}
