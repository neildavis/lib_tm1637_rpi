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
    m_pinDataMode = OUTPUT;
    pinMode(m_pinData, m_pinDataMode);
}

WiringPi::~WiringPi() {}

void WiringPi::setClock(PinDigitalState state) {
    digitalWrite(m_pinClk, state == PIN_HIGH ? HIGH : LOW);
}

void WiringPi::setData(PinDigitalState state) {
    if (OUTPUT != m_pinDataMode) {
        m_pinDataMode = OUTPUT;
        pinMode(m_pinData, m_pinDataMode);
    }
    digitalWrite(m_pinData, state == PIN_HIGH ? HIGH : LOW);
}

PinDigitalState WiringPi::getData() {
    if (INPUT != m_pinDataMode) {
        m_pinDataMode = INPUT;
        pinMode(m_pinData, m_pinDataMode);
        pullUpDnControl(m_pinData, PUD_UP);
    }
   return digitalRead(m_pinData) == HIGH ? PIN_HIGH : PIN_LOW;
}

void WiringPi::delayMicroseconds(int usecs) {
    ::delayMicroseconds(usecs);
}