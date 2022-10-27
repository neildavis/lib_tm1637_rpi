#include "gpioGPIOD.h"

#include <chrono>
#include <thread>

using namespace tm1637;

const char *kDefaultGpioChipName = "gpiochip0";
const char *kLineConsumer = "libTM1637Pi";

GPIOD::GPIOD(int pinClk, int pinData) {
    m_chip = gpiod_chip_open_by_name(kDefaultGpioChipName);
    m_lineClk = gpiod_chip_get_line(m_chip, pinClk);
    m_lineData = gpiod_chip_get_line(m_chip, pinData);
    gpiod_line_request_output(m_lineClk,  kLineConsumer, 0); // open CLK as LOW
    gpiod_line_request_output(m_lineData, kLineConsumer, 0); // open DIO as LOW
}

GPIOD::~GPIOD() {
    gpiod_line_release(m_lineClk);
    gpiod_line_release(m_lineData);
    gpiod_chip_close(m_chip);
}

void GPIOD::setClock(PinDigitalState state) {
    gpiod_line_set_value(m_lineClk, state);
}

void GPIOD::setData(PinDigitalState state) {
    gpiod_line_set_value(m_lineData, state);
}

void GPIOD::delayMicroseconds(int usecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}