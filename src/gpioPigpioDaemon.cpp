#include "gpioPigpioDaemon.h"

#include <dlfcn.h>
#include <stdexcept>
#include <sstream>

using namespace tm1637;

const char *kPigpiodLibSoName = "libpigpiod_if2.so.1";

PigpioDaemon::PigpioDaemon(int pinClk, int pinData) :
    m_pinClk(pinClk),
    m_pinData(pinData) {
    dynLoadLib();
    int ret = m_pigpio_start(NULL, NULL);
    if (ret < 0) {
        throwLibpigpiodReturnValue("pigpio_start", ret);
    }
    m_pi = ret;
    setPinMode(pinClk, PI_OUTPUT);
    setPinMode(pinData, PI_OUTPUT);
}

PigpioDaemon::~PigpioDaemon() {
    if (0 != m_pi) {
        m_pigpio_stop(m_pi);
    }
}

void PigpioDaemon::setClock(PinDigitalState state) {
    gpioWrite(m_pinClk, state);
}

void PigpioDaemon::setData(PinDigitalState state) {
    gpioWrite(m_pinData, state);
}

void PigpioDaemon::delayMicroseconds(int usecs) {
    m_time_sleep(usecs / 1000000);
}

void PigpioDaemon::dynLoadLib() {
    m_libHandle = dlopen(kPigpiodLibSoName, RTLD_LAZY);
    if (NULL == m_libHandle) { throw std::runtime_error(dlerror()); }
    m_pigpio_start = reinterpret_cast<int (*)(const char*, const char*)>(dlsym(m_libHandle, "pigpio_start"));
    if (NULL == m_pigpio_start) { throw std::runtime_error(dlerror()); }
    m_pigpio_stop = reinterpret_cast<void (*)(int)>(dlsym(m_libHandle, "pigpio_stop"));
    if (NULL == m_pigpio_start) { throw std::runtime_error(dlerror()); }
    m_set_mode = reinterpret_cast<int (*)(int, unsigned, unsigned)>(dlsym(m_libHandle, "set_mode"));
    if (NULL == m_set_mode) { throw std::runtime_error(dlerror()); }
    m_gpio_write = reinterpret_cast<int (*)(int, unsigned, unsigned)>(dlsym(m_libHandle, "gpio_write"));
    if (NULL == m_gpio_write) { throw std::runtime_error(dlerror()); }
    m_time_sleep = reinterpret_cast<void (*)(double)>(dlsym(m_libHandle, "time_sleep"));
    if (NULL == m_time_sleep) { throw std::runtime_error(dlerror()); }
}

void PigpioDaemon::throwLibpigpiodReturnValue(const char *fn, int ret) {
    std::ostringstream str_stream;
    str_stream << "ERROR: libpigpiod_if2 func " << fn << " returned " << ret;
    throw std::runtime_error(str_stream.str());
}

void PigpioDaemon::setPinMode(unsigned pinNum, unsigned mode) {
    int ret = m_set_mode(m_pi, pinNum, mode);
    if (0 != ret) {
        throwLibpigpiodReturnValue("set_mode", ret);
    }

}

void PigpioDaemon::gpioWrite(unsigned pinNum, PinDigitalState state) {
    int ret = m_gpio_write(m_pi, pinNum, state);
    if (0 != ret) {
        throwLibpigpiodReturnValue("gpio_write", ret);
    }
}


