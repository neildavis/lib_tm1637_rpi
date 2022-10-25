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
        int m_pinClk;
        int m_pinData;
        bool m_bcm;
    };
}
