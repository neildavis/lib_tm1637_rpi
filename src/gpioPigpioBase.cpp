#include "gpioPigpioBase.h"

#include <dlfcn.h>
#include <stdexcept>
#include <sstream>

using namespace tm1637;

PigpioBase::PigpioBase(int pinClk, int pinData) : 
    m_pinClk(pinClk),
    m_pinData(pinData) {
}

PigpioBase::~PigpioBase() {
    dlclose(m_libHandle);
}

void PigpioBase::initialize() {
    dynLoadLib();
    initializeGpio();
    setPinMode(m_pinClk, PI_OUTPUT);
    setPinMode(m_pinData, PI_OUTPUT);
}

void PigpioBase::deinitialize() {
    deinitializeGpio();
}

void PigpioBase::setClock(PinDigitalState state) {
    gpioWrite(m_pinClk, state);
}

void PigpioBase::setData(PinDigitalState state) {
    gpioWrite(m_pinData, state);
}

void PigpioBase::delayMicroseconds(int usecs) {
    m_time_sleep(usecs / 1000000);
}

void PigpioBase::throwLibpigpiodReturnValue(const char *fn, int ret) {
    std::ostringstream str_stream;
    str_stream << "ERROR: libpigpiod_if2 func " << fn << " returned " << ret;
    throw std::runtime_error(str_stream.str());
}
