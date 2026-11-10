#include "tm1637.h"
#include "gpioWiringPi.h"
#include "gpioGPIOD.h"
#include "gpioPigpioInterface.h"
#include "gpioPigpioDaemon.h"
#include "gpioCommands.h"
#include "gpioExecutor.h"
#include <memory>
#include <utility>

using namespace tm1637;

const uint8_t CMD_DISPLAY_OFF   = 0x80;
const uint8_t CMD_DISPLAY_ON    = 0x88; // + 0-7 for brightness
const uint8_t ADDR_AUTO         = 0x40;
const uint8_t ADDR_FIXED        = 0x44;
const uint8_t ADDR_C0H          = 0xc0; // + 0-3 for digits: C0H, C1H, C2H, C3H
const uint8_t BRIGHTNESS_MAX    = 0x07;

// Max integer literals for Device::showIntegerLiteral()
const int MAX_INT_RADIX_DEC     = 9999;
const int MAX_INT_RADIX_HEX     = 0xffff;

// Clock signal
const int64_t TM1637_DELAY_MS = 10;

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

namespace {
std::unique_ptr<MGPIO> createGPIO(int pinClk, int pinData, GPIOLib gpioLib) {
    switch (gpioLib) {
    case GpioWiringPi:
        return std::make_unique<WiringPi>(pinClk, pinData, false);
    case GpioWiringPiBCM:
        return std::make_unique<WiringPi>(pinClk, pinData, true);
    case GpioPigpioInterface:
        return std::make_unique<PigpioInterface>(pinClk, pinData);
    case GpioPigpioDaemon:
        return std::make_unique<PigpioDaemon>(pinClk, pinData);
    case GpioGPIOD:
    default:
        return std::make_unique<GPIOD>(pinClk, pinData);
    }
}
}

Device::Device(int pinClk, int pinData) : Device(pinClk, pinData, GpioGPIOD) {
}

Device::Device(int pinClk, int pinData, GPIOLib gpioLib)
    : Device(pinClk, pinData, gpioLib, DeviceOptions{}) {
}

Device::Device(int pinClk, int pinData, GPIOLib gpioLib, const DeviceOptions& options)
    : m_data{0,0,0,0}
    , m_brightness(CMD_DISPLAY_ON | BRIGHTNESS_MAX)
    , m_showColon(false) {
    m_executor = std::make_unique<GPIOExecutor>(m_gpio,
        [pinClk, pinData, gpioLib] { return createGPIO(pinClk, pinData, gpioLib); },
        options.execution);
}

Device::~Device() = default;

void Device::wait(std::chrono::milliseconds duration) {
    m_executor->wait(duration);
}

void Device::flush() {
    m_executor->flush();
}

void Device::displayOff() {
    m_brightness = CMD_DISPLAY_OFF;
    showCurrentData();
}

void Device::setBrightnessPercent(int brightnessPercent) {
    if (brightnessPercent <= 0) {
        m_brightness = CMD_DISPLAY_ON;
    } else if (brightnessPercent >= 100) {
        m_brightness = CMD_DISPLAY_ON | BRIGHTNESS_MAX;
    } else {
        m_brightness = CMD_DISPLAY_ON | (((int)((brightnessPercent * BRIGHTNESS_MAX / 100.0) + 0.5)) & 0x7);
    }
    showCurrentData();
}

void Device::clear() {
    m_data[0] = 0;
    m_data[1] = 0;
    m_data[2] = 0;
    m_data[3] = 0;
    showCurrentData();
}

void Device::setColon(bool showColon) {
    m_showColon = showColon;
    showCurrentData();
}

void Device::showRawDigits(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
    m_data[0] = d1;
    m_data[1] = d2;
    m_data[2] = d3;
    m_data[3] = d4;
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

void Device::showIntegers(int int1, int int2, int int3, int int4) {
    int integers[] = {int1, int2, int3, int4};
    showIntegers(integers);
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
    const int &maxIntLit = (radix == RadixDecimal) ? MAX_INT_RADIX_DEC : MAX_INT_RADIX_HEX;
    if (intLit < 0 || intLit > maxIntLit) {
        return;
    }
    for (int i = 3; i >= 0; i--) {
        m_data[i] = digits[(intLit % radix) & 0xf];
        intLit /= radix;
    }
    showCurrentData();
}

namespace {
// Use these helpers with either the real backend or a recording command batch.
void protocolStart(MGPIO& gpio) {
    gpio.setClock(PIN_HIGH);
    gpio.setData(PIN_HIGH);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setData(PIN_LOW);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setClock(PIN_LOW);
}

void protocolStop(MGPIO& gpio) {
    gpio.setData(PIN_LOW);
    gpio.setClock(PIN_HIGH);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setData(PIN_HIGH);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setData(PIN_LOW);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setClock(PIN_LOW);
}

void protocolBreak(MGPIO& gpio) {
    protocolStop(gpio);
    protocolStart(gpio);
}

void protocolWriteByte(MGPIO& gpio, uint8_t data) {
    // Write bits in order LSB->MSB
    for (int i = 0; i < 8; i++) {
        gpio.setClock(PIN_LOW); // CLK falling edge for the previous iteration
        gpio.delayMicroseconds(TM1637_DELAY_MS);
        gpio.setData(data & 0x1 ? PIN_HIGH : PIN_LOW);
        gpio.setClock(PIN_HIGH);
        gpio.delayMicroseconds(TM1637_DELAY_MS);
        data >>= 1;
    }
    // This is the 8th bit CLK falling edge
    gpio.setClock(PIN_LOW);
    // Delay for ACK
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setClock(PIN_HIGH);
    gpio.delayMicroseconds(TM1637_DELAY_MS);
    gpio.setClock(PIN_LOW);

    gpio.setData(PIN_LOW);
}
}

// Protected helpers follow the same execution mode as complete display updates.
void Device::start() const {
    GPIOCommandBatch commands(6);
    protocolStart(commands);
    m_executor->submit(std::move(commands));
}

void Device::stop() const {
    GPIOCommandBatch commands(8);
    protocolStop(commands);
    m_executor->submit(std::move(commands));
}

void Device::br() const {
    GPIOCommandBatch commands(14);
    protocolBreak(commands);
    m_executor->submit(std::move(commands));
}

void Device::writeByte(uint8_t data) const {
    GPIOCommandBatch commands(46);
    protocolWriteByte(commands, data);
    m_executor->submit(std::move(commands));
}

void Device::showCurrentData() {
    applyColonMask();
    // A full update currently takes 364 operations. Reserve space up front so
    // recording normally needs just one allocation; the batch can grow if needed.
    GPIOCommandBatch commands(364);
    protocolStart(commands);
    protocolWriteByte(commands, ADDR_AUTO);
    protocolBreak(commands);
    protocolWriteByte(commands, ADDR_C0H);
    for (int i = 0; i < 4; i++) {
        protocolWriteByte(commands, m_data[i]);
    }
    protocolBreak(commands);
    protocolWriteByte(commands, m_brightness);
    protocolStop(commands);
    m_executor->submit(std::move(commands));
}

void Device::showDigitAtPos(int pos) {
    if (pos < 0 || pos > 3) {
        return;
    }
    if (1 == pos) {
        // Retain colon mask
        applyColonMask();
    }
    GPIOCommandBatch commands(226); // Complete single-digit update.
    protocolStart(commands);
    protocolWriteByte(commands, ADDR_FIXED);
    protocolBreak(commands);
    protocolWriteByte(commands, ADDR_C0H | (uint8_t)pos);
    protocolWriteByte(commands, m_data[pos]);
    protocolBreak(commands);
    protocolWriteByte(commands, m_brightness);
    protocolStop(commands);
    m_executor->submit(std::move(commands));
}

void Device::applyColonMask() {
    if (m_showColon) {
        m_data[1] |= COLON_MASK;
    } else {
        m_data[1] &= ~COLON_MASK;
    }
}
