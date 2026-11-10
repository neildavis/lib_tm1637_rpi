#pragma once

#include "gpio.h"

#include <cstddef>
#include <vector>

namespace tm1637 {

// Commands own their arguments; they never refer to Device state or input arrays.
struct GPIOCommand {
    enum class Kind { Initialize, Deinitialize, SetClock, SetData, DelayMicroseconds };

    Kind kind;
    int argument;

    void execute(MGPIO& gpio) const;
};

// A recording GPIO target lets protocol generation use the same interface as
// direct execution. A batch is submitted only once its sequence is complete.
class GPIOCommandBatch : public MGPIO {
public:
    explicit GPIOCommandBatch(std::size_t capacity = 0);
    GPIOCommandBatch(const GPIOCommandBatch&) = delete;
    GPIOCommandBatch& operator=(const GPIOCommandBatch&) = delete;
    GPIOCommandBatch(GPIOCommandBatch&&) noexcept;
    GPIOCommandBatch& operator=(GPIOCommandBatch&&) noexcept;

    void initialize() override;
    void deinitialize() override;
    void setClock(PinDigitalState state) override;
    void setData(PinDigitalState state) override;
    void delayMicroseconds(int usecs) override;

    void drain(MGPIO& gpio);

private:
    void append(GPIOCommand::Kind kind, int argument = 0);

    std::vector<GPIOCommand> m_commands;
};

}
