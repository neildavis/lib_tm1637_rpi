#pragma once

#include "gpio.h"
#include "gpioPigpioBase.h"

namespace tm1637 {
    
    class PigpioInterface : public PigpioBase {
    public:
        PigpioInterface(int pinClk, int pinData);
    private:
        // Internal template method overrides
        virtual void dynLoadLib() override;
        virtual void initializeGpio() override;
        virtual void deinitializeGpio() override;
        virtual void setPinMode(unsigned pinNum, unsigned mode) override;
        virtual void gpioWrite(unsigned pinNum, PinDigitalState state) override;
    private:
        // These are func pointers we assign from the symbols in libpigpiod_if2.so.1
        int (*m_gpioInitialise)();
        void (*m_gpioTerminate)();
        int (*m_gpioSetMode)(unsigned gpio, unsigned mode);
        int (*m_gpioWrite)(unsigned gpio, unsigned level);
   };
}
