#include "gpioPigpioBase.h"

#include <dlfcn.h>
#include <stdexcept>
#include <sstream>

using namespace tm1637;

const char *kPigpiodLibSoName = "libpigpiod_if2.so.1";

PigpioBase::PigpioBase(int pinClk, int pinData) : 
    m_pinClk(pinClk),
    m_pinData(pinData) {
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

void PigpioBase::dynLoadLib() {
    m_libHandle = dlopen(kPigpiodLibSoName, RTLD_LAZY);
    m_time_sleep = reinterpret_cast<void (*)(double)>(dlsym(m_libHandle, "time_sleep"));
    if (NULL == m_time_sleep) { throw std::runtime_error(dlerror()); }
}

void PigpioBase::throwLibpigpiodReturnValue(const char *fn, int ret) {
    std::ostringstream str_stream;
    str_stream << "ERROR: libpigpiod_if2 func " << fn << " returned " << ret;
    throw std::runtime_error(str_stream.str());
}
