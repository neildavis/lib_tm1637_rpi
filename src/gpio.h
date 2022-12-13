#pragma once

namespace tm1637 {

    typedef enum {
        PIN_LOW = 0,
        PIN_HIGH,
    } PinDigitalState;

    /**
     * MGPIO is an abstract base class defining the interface for GPIO access
    */
    class MGPIO {
    public:
        virtual ~MGPIO() = default;
        // API
        virtual void initialize() = 0;
        virtual void deinitialize() = 0;
        virtual void setClock(PinDigitalState state) = 0;
        virtual void setData(PinDigitalState state) = 0;
        virtual void delayMicroseconds(int usecs) = 0;
    };
}