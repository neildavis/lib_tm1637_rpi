#include "gpioWiringPi.h"
#include <wiringPi.h>

using namespace tm1637;

WiringPi::WiringPi(int pinClk, int pinData, bool useBCM)
    : m_pinClk(pinClk)
    , m_pinData(pinData)
    , m_bcm(useBCM) {
    if (m_bcm) {
        wiringPiSetupGpio();
    } else {
        wiringPiSetup();
    }
    pinMode(m_pinClk, OUTPUT);
    pinMode(m_pinData, OUTPUT);
}

WiringPi::~WiringPi() {}

void WiringPi::setClock(PinDigitalState state) {
    digitalWrite(m_pinClk, state == PIN_HIGH ? HIGH : LOW);
}

void WiringPi::setData(PinDigitalState state) {
    digitalWrite(m_pinData, state == PIN_HIGH ? HIGH : LOW);
}

void WiringPi::delayMicroseconds(int usecs) {
    ::delayMicroseconds(usecs);
}