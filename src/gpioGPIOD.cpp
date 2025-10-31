#include "gpioGPIOD.h"

#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <string>

#include "gpioGPIODv1.h"
#include "gpioGPIODv2.h"

using namespace tm1637;

GPIOD::GPIOD(int pinClk, int pinData) :
    m_pinClk(pinClk),
    m_pinData(pinData) {
}

void GPIOD::initialize() {
    int soVersion = dynLoadGpiodLib(); // throws if unsuccessful
    // Initialize the correct MGPIO proxy for the libgpiod SO version loaded
    // Note: Due to legacy reasons, SO.3 is lib version >= 2.1 and SO.2 is >= v1.5.1 
    switch (soVersion)
    {
    case 3:
        m_gpio = std::make_unique<GPIODv2>(m_pinClk, m_pinData, m_libHandle);
        break;
    case 2:
        m_gpio = std::make_unique<GPIODv1>(m_pinClk, m_pinData, m_libHandle);
        break;
    default:
        // Should never get here since dynLoadGpiodLib() will throw if not returning
        //  supported SO version on a successful dyn lib load
        throw std::runtime_error("Error: Unsupported libgpiod SO version");
        break;
    }
    // If we get here, m_gpio is assumed to be valid since dynLoadGpiodLib will throw if not.
    m_gpio->initialize();
}

void GPIOD::deinitialize() {
    m_gpio->deinitialize();
    dlclose(m_libHandle);
}

void GPIOD::setClock(PinDigitalState state) {
    m_gpio->setClock(state);
}

void GPIOD::setData(PinDigitalState state) {
    m_gpio->setData(state);
}

void GPIOD::delayMicroseconds(int usecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}

int GPIOD::dynLoadGpiodLib() {
    // Attempt to open libgpiod.so.N in this order of 'N'
    std::vector<int> soVersions = {
        3,  // libgpiod3 in Debian 13 'Trixie' +
        2   // libgpiod2 in Debian 12 'Bookworm' (and earlier)
    };
    for (auto it = soVersions.begin(); it != soVersions.end(); it++) {
        std::ostringstream oss;
        oss << "libgpiod.so." << *it;
        m_libHandle = dlopen(oss.str().c_str(), RTLD_LAZY);
        if (m_libHandle) {
            return *it;
        }
    }
    std::ostringstream oss;
    oss << "Unable to load any supported SO version of libgpiod.so. Tried versions: ";
    for (auto it = soVersions.begin(); it != soVersions.end(); it++) {
        oss << *it;
        if (it < soVersions.end() - 1) {
            oss << ", ";
        }
    }

    throw std::runtime_error(oss.str().c_str());
    return -1;  // never get here due to exception thrown, but satisifes the compiler
}
