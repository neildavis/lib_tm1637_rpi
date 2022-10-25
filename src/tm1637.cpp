#include "tm1637.h"
#include "gpioWiringPi.h"

#include <chrono>
#include <thread>

using namespace tm1637;

const uint8_t CMD_DISPLAY_OFF   = 0x80;
const uint8_t CMD_DISPLAY_ON    = 0x88; // + 0-7 for brightness
const uint8_t ADDR_AUTO         = 0x40;
const uint8_t ADDR_FIXED        = 0x44;
const uint8_t ADDR_C0H          = 0xc0; // + 0-3 for digits: C0H, C1H, C2H, C3H
const uint8_t BRIGHTNESS_MAX    = 0x07;

const uint8_t digits[] = {
        0b0111111, // 0
        0b0000110, // 1
        0b1011011, // 2
        0b1001111, // 3
        0b1100110, // 4
        0b1101101, // 5
        0b1111101, // 6
        0b0000111, // 7
        0b1111111, // 8
        0b1101111, // 9
        0b1110111, // A
        0b1111100, // b
        0b0111001, // C
        0b1011110, // d
        0b1111001, // E
        0b1110001  // F
};
const uint8_t COLON_MASK = 0x80;    // 0b10000000

Device::Device(int pinClk, int pinData, GPIOLib gpioLib) 
    : m_data{0,0,0,0}
    , m_brightness(BRIGHTNESS_MAX) {
    switch (gpioLib) {
    case GpioWiringPi:
        m_gpio = new WiringPi(pinClk, pinData, false);
        break;
    case GpioWiringPiBCM:
        m_gpio = new WiringPi(pinClk, pinData, true);
        break;
    default:
        // TODO: error handling
        throw(1);
        break;
    }
    m_gpio->setClock(PIN_LOW);
    m_gpio->setData(PIN_LOW);
}

Device::~Device() {
    delete m_gpio;
}

void Device::displayOff() {
    m_brightness = CMD_DISPLAY_OFF;
    showCurrentData();
}

void Device::setBrightnessPercent(int brightnessPercent) {
    if (brightnessPercent < 0) {
        m_brightness = 0;
    } else if (brightnessPercent > 100) {
        m_brightness = BRIGHTNESS_MAX;
    } else {
        m_brightness = CMD_DISPLAY_ON | (((int)((brightnessPercent * BRIGHTNESS_MAX / 100.0) + 0.5)) & 0x7);
    }
    showCurrentData();
}

void Device::clear() {
    m_data[0] = m_data[1] = m_data[2] = m_data[3] = 0;
    showCurrentData();
}

void Device::setColon(bool showColon) {
    if (showColon) {
        m_data[0] |= COLON_MASK;
        m_data[1] |= COLON_MASK;
        m_data[2] |= COLON_MASK;
        m_data[3] |= COLON_MASK;
    } else {
        m_data[0] &= ~COLON_MASK;
        m_data[1] &= ~COLON_MASK;
        m_data[2] &= ~COLON_MASK;
        m_data[3] &= ~COLON_MASK;
    }
    showCurrentData();
}

void Device::showRawDigits(const uint8_t *digits) {
    for (int i = 0; i < 4; i++) {
        m_data[i] = digits[i];
    }
    showCurrentData();
}

void Device::showRawDigit(int pos, uint8_t digit) {
    if (pos < 0 || pos > 3) {
        return;
    }
    m_data[pos] = digit;
    showDigitAtPos(pos);
}

void Device::showIntegers(const int *integers) {
    for (int i = 0; i < 4; i++) {
        int thisInt = integers[i];
        if (thisInt < 0 || thisInt  > 0xf) {
            m_data[i] = 0;  // invalid int
        } else {
            m_data[i] = digits[thisInt];
        }
     }
     showCurrentData();
}

void Device::showInteger(int pos, int integer) {
    if (pos < 0 || pos > 3) {
        return;
    }
    if (integer < 0 || integer > 0xf) {
        m_data[pos] = 0;  // invalid int
    } else {
        m_data[pos] = digits[integer];
    }
    showDigitAtPos(pos);
}

void Device::showIntegerLiteral(int intLit, Radix radix) {
    if (intLit < 0 || intLit >= radix^4) {
        return;
    }
    for (int i = 3; i >= 0; i++) {
        m_data[i] = digits[(intLit % radix) & 0xf];
        intLit /= radix;
    }
    showCurrentData();
}

void Device::start() const {
    m_gpio->setClock(PIN_HIGH);
    m_gpio->setData(PIN_HIGH);
    m_gpio->setData(PIN_LOW);
    m_gpio->setClock(PIN_LOW);
}

void Device::stop() const {
    m_gpio->setClock(PIN_LOW);
    m_gpio->setData(PIN_LOW);
    m_gpio->setClock(PIN_HIGH);
    m_gpio->setData(PIN_HIGH);
}

void Device::writeByte(uint8_t data) const {
    // Write bits in order LSB->MSB
    for (int i = 0; i < 8; i++) {
        m_gpio->setClock(PIN_LOW);
        m_gpio->setData(data & 0x1 ? PIN_HIGH : PIN_LOW);
        m_gpio->setClock(PIN_HIGH);
        data >>= 1;
    }
    // Wait for ACK
    m_gpio->setClock(PIN_LOW);
    m_gpio->setData(PIN_HIGH);
    m_gpio->setClock(PIN_HIGH);
    while(m_gpio->getData() == PIN_HIGH) {
        m_gpio->delayMicroseconds(1000);
    }
 }

void Device::showCurrentData() const {
    start();
    writeByte(ADDR_AUTO);
    br();
    writeByte(ADDR_C0H);
    for (int i = 0; i < 4; i++) {
        writeByte(m_data[i]);
    }
    br();
    writeByte(m_brightness);
    stop();
}

void Device::showDigitAtPos(int pos) {
    if (pos < 0 || pos > 3) {
        return;
    }
    start();
    writeByte(ADDR_FIXED);
    br();
    writeByte(ADDR_C0H | (uint8_t)pos);
    writeByte(m_data[pos]);
    br();
    writeByte(m_brightness);
    stop();
}
