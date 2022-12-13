#include "gpioPigpioInterface.h"

#include <dlfcn.h>
#include <stdexcept>

using namespace tm1637;

const char *kPigpioLibSoName = "libpigpio.so.1";

PigpioInterface::PigpioInterface(int pinClk, int pinData) :
    PigpioBase(pinClk, pinData) {
}

void PigpioInterface::dynLoadLib() {
    m_libHandle = dlopen(kPigpioLibSoName, RTLD_LAZY);
    m_time_sleep = reinterpret_cast<void (*)(double)>(dlsym(m_libHandle, "time_sleep"));
    if (NULL == m_time_sleep) { throw std::runtime_error(dlerror()); }
    m_gpioInitialise = reinterpret_cast<int (*)()>(dlsym(m_libHandle, "gpioInitialise"));
    if (NULL == m_gpioInitialise) { throw std::runtime_error(dlerror()); }
    m_gpioTerminate = reinterpret_cast<void (*)()>(dlsym(m_libHandle, "gpioTerminate"));
    if (NULL == m_gpioTerminate) { throw std::runtime_error(dlerror()); }
    m_gpioSetMode = reinterpret_cast<int (*)(unsigned, unsigned)>(dlsym(m_libHandle, "gpioSetMode"));
    if (NULL == m_gpioSetMode) { throw std::runtime_error(dlerror()); }
    m_gpioWrite = reinterpret_cast<int (*)(unsigned, unsigned)>(dlsym(m_libHandle, "gpioWrite"));
    if (NULL == m_gpioWrite) { throw std::runtime_error(dlerror()); }
}

void PigpioInterface::initializeGpio() {
    int ret = m_gpioInitialise();
    if (ret < 0) {
        throwLibpigpiodReturnValue("gpioInitialise", ret);
    }
}

void PigpioInterface::deinitializeGpio() {
    m_gpioTerminate();
}

void PigpioInterface::setPinMode(unsigned pinNum, unsigned mode) {
    int ret = m_gpioSetMode(pinNum, mode);
    if (0 != ret) {
        throwLibpigpiodReturnValue("gpioSetMode", ret);
    }
}

void PigpioInterface::gpioWrite(unsigned pinNum, PinDigitalState state) {
    int ret = m_gpioWrite(pinNum, state);
    if (0 != ret) {
        throwLibpigpiodReturnValue("gpioWrite", ret);
    }
}
