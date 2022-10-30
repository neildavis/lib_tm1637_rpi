#pragma once

#include "gpio.h"

namespace tm1637 {
    
    class WiringPi : public MGPIO {
    public:
        WiringPi(int pinClk, int pinData, bool useBCM);
        ~WiringPi();
        // API
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        void dynLoadWiringPiLib();
    private:
        // These are func pointers we assign from the symbols in libwiringPi.so
        int (*m_wiringPiSetup)();
        int (*m_wiringPiSetupGpio)();
        int (*m_pinMode)(int pin, int mode);
        void (*m_digitalWrite)(int pin, int value);
        void (*m_delayMicroseconds)(unsigned int howLong);
    private:
        void *m_libHandle;
        int m_pinClk;
        int m_pinData;
        bool m_bcm;
    };
}
