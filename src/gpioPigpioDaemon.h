#pragma once

#include "gpio.h"
#include <pigpiod_if2.h>

namespace tm1637 {
    
    class PigpioDaemon : public MGPIO {
    public:
        PigpioDaemon(int pinClk, int pinData);
        virtual ~PigpioDaemon();
        // API
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        // Internal helper functions
        void dynLoadLib();
        void throwLibpigpiodReturnValue(const char *fn, int ret);
        void setPinMode(unsigned pinNum, unsigned mode);
        void gpioWrite(unsigned pinNum, PinDigitalState state);
    private:
        // These are func pointers we assign from the symbols in libpigpiod_if2.so.1
        int (*m_pigpio_start)(const char *addrStr, const char *addrPortStr);
        void (*m_pigpio_stop)(int pi);
        int (*m_set_mode)(int pi, unsigned gpio, unsigned mode);
        int (*m_gpio_write)(int pi, unsigned gpio, unsigned level);
        void (*m_time_sleep)(double seconds);
    private:
        void *m_libHandle;
        int m_pi;
        unsigned m_pinClk;
        unsigned m_pinData;
   };
}
