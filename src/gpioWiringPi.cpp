#include "gpioWiringPi.h"
#include <wiringPi.h>

#include <dlfcn.h>
#include <stdexcept>

using namespace tm1637;

// wiringPi doesn't seem to install with a SONAME with a single major version, e.g. 'libwiringPi.so.2'
// but since it's largely deprecated, we'll just assume the non-version name will be v2.x
const char *kWiringPiLibSoName = "libwiringPi.so";  

WiringPi::WiringPi(int pinClk, int pinData, bool useBCM)
    : m_pinClk(pinClk)
    , m_pinData(pinData)
    , m_bcm(useBCM) {
}

void WiringPi::initialize() {
    dynLoadWiringPiLib();

    if (m_bcm) {
        m_wiringPiSetupGpio();
    } else {
        m_wiringPiSetup();
    }
    m_pinMode(m_pinClk, OUTPUT);
    m_pinMode(m_pinData, OUTPUT);
}

void WiringPi::deinitialize() {
    dlclose(m_libHandle);
}

void WiringPi::setClock(PinDigitalState state) {
    m_digitalWrite(m_pinClk, state == PIN_HIGH ? HIGH : LOW);
}

void WiringPi::setData(PinDigitalState state) {
    m_digitalWrite(m_pinData, state == PIN_HIGH ? HIGH : LOW);
}

void WiringPi::delayMicroseconds(int usecs) {
    m_delayMicroseconds(usecs);
}

void WiringPi::dynLoadWiringPiLib() {
    m_libHandle = dlopen(kWiringPiLibSoName, RTLD_LAZY);
    if (NULL == m_libHandle) { throw std::runtime_error(dlerror()); }
    m_wiringPiSetup = reinterpret_cast<int(*)()>(dlsym(m_libHandle, "wiringPiSetup"));
    if (NULL == m_wiringPiSetup) { throw std::runtime_error(dlerror()); }
    m_wiringPiSetupGpio = reinterpret_cast<int(*)()>(dlsym(m_libHandle, "wiringPiSetupGpio"));
    if (NULL == m_wiringPiSetupGpio) { throw std::runtime_error(dlerror()); }
    m_pinMode = reinterpret_cast<int(*)(int, int)>(dlsym(m_libHandle, "pinMode"));
    if (NULL == m_pinMode) { throw std::runtime_error(dlerror()); }
    m_digitalWrite = reinterpret_cast<void(*)(int, int)>(dlsym(m_libHandle, "digitalWrite"));
    if (NULL == m_digitalWrite) { throw std::runtime_error(dlerror()); }
    m_delayMicroseconds = reinterpret_cast<void(*)(unsigned int)>(dlsym(m_libHandle, "delayMicroseconds"));
    if (NULL == m_delayMicroseconds) { throw std::runtime_error(dlerror()); }
}
