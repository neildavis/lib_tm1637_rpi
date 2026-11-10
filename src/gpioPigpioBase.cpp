#include "gpioPigpioBase.h"

#include <dlfcn.h>
#include <stdexcept>
#include <sstream>

using namespace tm1637;

#define PI_OUTPUT 1

PigpioBase::PigpioBase(int pinClk, int pinData) : 
    m_pinClk(pinClk),
    m_pinData(pinData) {
}

PigpioBase::~PigpioBase() {
    if (m_libHandle) {
        dlclose(m_libHandle);
    }
}

void PigpioBase::initialize() {
    dynLoadLib();
    initializeGpio();
    m_initialized = true;
    setPinMode(m_pinClk, PI_OUTPUT);
    setPinMode(m_pinData, PI_OUTPUT);
}

void PigpioBase::deinitialize() {
    if (m_initialized) {
        m_initialized = false;
        deinitializeGpio();
    }
    if (m_libHandle) {
        dlclose(m_libHandle);
        m_libHandle = nullptr;
    }
}

void PigpioBase::setClock(PinDigitalState state) {
    gpioWrite(m_pinClk, state);
}

void PigpioBase::setData(PinDigitalState state) {
    gpioWrite(m_pinData, state);
}

void PigpioBase::delayMicroseconds(int usecs) {
    m_time_sleep(usecs / 1000000.0);
}

void PigpioBase::throwLibpigpiodReturnValue(const char *fn, int ret) {
    std::ostringstream str_stream;
    str_stream << "ERROR: libpigpiod_if2 func " << fn << " returned " << ret;
    throw std::runtime_error(str_stream.str());
}
