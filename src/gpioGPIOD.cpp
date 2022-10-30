#include "gpioGPIOD.h"

#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <stdexcept>

using namespace tm1637;

const char *kGpiodLibSoName = "libgpiod.so.2";
const char *kDefaultGpioChipName = "gpiochip0";
const char *kLineConsumer = "libTM1637Pi";

GPIOD::GPIOD(int pinClk, int pinData) {
    dynLoadGpiodLib();

    m_chip = m_gpiod_chip_open_by_name(kDefaultGpioChipName);
    m_lineClk = m_gpiod_chip_get_line(m_chip, pinClk);
    m_lineData = m_gpiod_chip_get_line(m_chip, pinData);
    m_gpiod_line_request_output(m_lineClk,  kLineConsumer, 0); // open CLK as LOW
    m_gpiod_line_request_output(m_lineData, kLineConsumer, 0); // open DIO as LOW
}

GPIOD::~GPIOD() {
    m_gpiod_line_release(m_lineClk);
    m_gpiod_line_release(m_lineData);
    m_gpiod_chip_close(m_chip);
    dlclose(m_libHandle);
}

void GPIOD::setClock(PinDigitalState state) {
    m_gpiod_line_set_value(m_lineClk, state);
}

void GPIOD::setData(PinDigitalState state) {
    m_gpiod_line_set_value(m_lineData, state);
}

void GPIOD::delayMicroseconds(int usecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}

void GPIOD::dynLoadGpiodLib() {
    m_libHandle = dlopen(kGpiodLibSoName, RTLD_LAZY);
    if (NULL == m_libHandle) { throw std::runtime_error(dlerror()); }
    m_gpiod_chip_open_by_name = reinterpret_cast<gpiod_chip* (*)(const char*)>(dlsym(m_libHandle, "gpiod_chip_open_by_name"));
    if (NULL == m_gpiod_chip_open_by_name) { throw std::runtime_error(dlerror()); }
    m_gpiod_chip_close = reinterpret_cast<void (*)(gpiod_chip*)>(dlsym(m_libHandle, "gpiod_chip_close"));
    if (NULL == m_gpiod_chip_close) { throw std::runtime_error(dlerror()); }
    m_gpiod_chip_get_line = reinterpret_cast<gpiod_line* (*)(gpiod_chip*, unsigned int)>(dlsym(m_libHandle, "gpiod_chip_get_line"));
    if (NULL == m_gpiod_chip_get_line) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_release = reinterpret_cast<void (*)(gpiod_line*)>(dlsym(m_libHandle, "gpiod_line_release"));
    if (NULL == m_gpiod_line_release) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_request_output = reinterpret_cast<int (*)(gpiod_line*, const char*, int)>(dlsym(m_libHandle, "gpiod_line_request_output"));
    if (NULL == m_gpiod_line_request_output) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_set_value = reinterpret_cast<int (*)(gpiod_line*, int)>(dlsym(m_libHandle, "gpiod_line_set_value"));
    if (NULL == m_gpiod_line_set_value) { throw std::runtime_error(dlerror()); }
}
