#pragma once

#include "gpio.h"
#include <pigpiod_if2.h>

namespace tm1637 {
    
    class PigpioBase : public MGPIO {
    public:
        ~PigpioBase();
        // API
        virtual void initialize() override;
        virtual void deinitialize() override;
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    protected:
        PigpioBase(int pinClk, int pinData);
        // Internal helper functions
        void throwLibpigpiodReturnValue(const char *fn, int ret);
        // Internal template methods - overridden by subclasses
        virtual void dynLoadLib() = 0;
        virtual void initializeGpio() = 0;
        virtual void deinitializeGpio() = 0;
        virtual void setPinMode(unsigned pinNum, unsigned mode) = 0;
        virtual void gpioWrite(unsigned pinNum, PinDigitalState state) = 0;
    protected:
        // These are func pointers we assign from the symbols in libpigpiod_if2.so.1
        void (*m_time_sleep)(double seconds);
    protected:
        // Member data
        void *m_libHandle;
        unsigned m_pinClk;
        unsigned m_pinData;
   };
}
