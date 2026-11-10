#include "gpioCommands.h"

using namespace tm1637;

void GPIOCommand::execute(MGPIO& gpio) const {
    switch (kind) {
    case Kind::Initialize:
        gpio.initialize();
        break;
    case Kind::Deinitialize:
        gpio.deinitialize();
        break;
    case Kind::SetClock:
        gpio.setClock(static_cast<PinDigitalState>(argument));
        break;
    case Kind::SetData:
        gpio.setData(static_cast<PinDigitalState>(argument));
        break;
    case Kind::DelayMicroseconds:
        gpio.delayMicroseconds(argument);
        break;
    }
}

GPIOCommandBatch::GPIOCommandBatch(std::size_t capacity) {
    m_commands.reserve(capacity);
}

GPIOCommandBatch::GPIOCommandBatch(GPIOCommandBatch&&) noexcept = default;
GPIOCommandBatch& GPIOCommandBatch::operator=(GPIOCommandBatch&&) noexcept = default;

void GPIOCommandBatch::initialize() {
    append(GPIOCommand::Kind::Initialize);
}

void GPIOCommandBatch::deinitialize() {
    append(GPIOCommand::Kind::Deinitialize);
}

void GPIOCommandBatch::setClock(PinDigitalState state) {
    append(GPIOCommand::Kind::SetClock, state);
}

void GPIOCommandBatch::setData(PinDigitalState state) {
    append(GPIOCommand::Kind::SetData, state);
}

void GPIOCommandBatch::delayMicroseconds(int usecs) {
    append(GPIOCommand::Kind::DelayMicroseconds, usecs);
}

void GPIOCommandBatch::drain(MGPIO& gpio) {
    // Remove the batch before executing so an exception cannot leave stale
    // operations queued for a later sequence. Errors propagate to the caller.
    std::vector<GPIOCommand> commands;
    commands.swap(m_commands);
    for (const auto& command : commands) {
        command.execute(gpio);
    }
}

void GPIOCommandBatch::append(GPIOCommand::Kind kind, int argument) {
    m_commands.push_back({kind, argument});
}
