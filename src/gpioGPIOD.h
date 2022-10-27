#pragma once

#include "gpio.h"
#include <gpiod.h>

namespace tm1637 {
    
    class GPIOD : public MGPIO {
    public:
        GPIOD(int pinClk, int pinData);
        virtual ~GPIOD();
        // API
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        gpiod_line *m_lineClk;
        gpiod_line *m_lineData;
        gpiod_chip *m_chip;
   };
}
