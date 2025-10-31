#pragma once

#include "gpio.h"
#include <memory>

namespace tm1637 {
    
    class GPIOD : public MGPIO {
    public:
        GPIOD(int pinClk, int pinData);
        // API
        virtual void initialize() override;
        virtual void deinitialize() override;
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        int dynLoadGpiodLib();
    private:
        int m_pinClk;
        int m_pinData;
        void *m_libHandle;
        std::unique_ptr<MGPIO> m_gpio; // proxied for GPIODv1 or GPIODv2
    };
}
