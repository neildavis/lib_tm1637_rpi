#include "tm1637.h"
#include "fakeWiringPi.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr int clockPin = 23;
constexpr int dataPin = 24;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Serialize every backend call, including its arguments, for exact comparison
// with traces captured from the original, immediate-execution implementation.
std::string trace() {
    std::ostringstream output;
    for (const auto& call : fake_wiringpi::calls()) {
        require(call.thread == std::this_thread::get_id(),
                "GPIO operation did not execute on the calling thread");
        switch (call.kind) {
        case fake_wiringpi::Kind::Setup: output << "setup;"; break;
        case fake_wiringpi::Kind::SetupBCM: output << "bcm;"; break;
        case fake_wiringpi::Kind::PinMode:
            output << "mode(" << call.first << ',' << call.second << ");";
            break;
        case fake_wiringpi::Kind::Write:
            require(call.first == clockPin || call.first == dataPin,
                    "Unexpected GPIO pin");
            require(call.second == 0 || call.second == 1,
                    "Unexpected GPIO state");
            output << (call.first == clockPin ? 'C' : 'D') << call.second;
            break;
        case fake_wiringpi::Kind::Delay:
            output << '[' << call.first << ']';
            break;
        }
    }
    return output.str();
}

class ProtocolDevice : public tm1637::Device {
public:
    ProtocolDevice() : Device(clockPin, dataPin, tm1637::GpioWiringPi) {}
    using Device::start;
    using Device::stop;
    using Device::br;
    using Device::writeByte;
};
}

int main(int argc, char** argv) {
    try {
        require(argc == 2, "Usage: tm1637_device_tests <trace-file|--record>");
        const bool record = std::string(argv[1]) == "--record";
        std::map<std::string, std::string> expected;
        if (!record) {
            std::ifstream input(argv[1]);
            require(input.good(), "Cannot read protocol traces");
            std::string name, value;
            while (std::getline(input, name) && std::getline(input, value)) {
                require(expected.emplace(name, value).second,
                        "Duplicate trace: " + name);
            }
        }
        std::size_t checked = 0;
        auto check = [&](const std::string& name) {
            const auto actual = trace();
            if (record) {
                std::cout << name << '\n' << actual << '\n';
            } else {
                const auto found = expected.find(name);
                require(found != expected.end(), "Missing trace: " + name);
                require(found->second == actual, "GPIO trace differs: " + name);
            }
            ++checked;
        };
        auto update = [&](const std::string& name, const std::function<void()>& action) {
            fake_wiringpi::reset();
            action();
            // The complete trace must already be present when the call returns.
            check(name);
        };

        fake_wiringpi::reset();
        {
            ProtocolDevice device;
            check("initialize-wiringpi");
            update("raw-digits", [&] { device.showRawDigits(0x00, 0xff, 0x55, 0xaa); });
            const uint8_t raw[] = {0xff, 0x00, 0xaa, 0x55};
            update("raw-digits-array", [&] { device.showRawDigits(raw); });
            update("colon-on", [&] { device.setColon(true); });
            update("single-digit-colon", [&] { device.showRawDigit(1, 0x01); });
            update("single-digit-last", [&] { device.showRawDigit(3, 0xa5); });
            update("clear-with-colon", [&] { device.clear(); });
            update("colon-off", [&] { device.setColon(false); });
            update("integers", [&] { device.showIntegers(0, 9, 10, 15); });
            const int integers[] = {-1, 16, 2, 3};
            update("integers-array-invalid", [&] { device.showIntegers(integers); });
            update("single-integer", [&] { device.showInteger(0, 14); });
            update("single-integer-invalid", [&] { device.showInteger(2, -1); });
            update("literal-decimal", [&] { device.showIntegerLiteral(1234); });
            update("literal-hex", [&] { device.showIntegerLiteral(0xabcd, tm1637::RadixHex); });
            update("brightness-low", [&] { device.setBrightnessPercent(-10); });
            update("brightness-middle", [&] { device.setBrightnessPercent(50); });
            update("brightness-high", [&] { device.setBrightnessPercent(110); });
            update("display-off", [&] { device.displayOff(); });
            update("clear-display-off", [&] { device.clear(); });
            update("invalid-positions-and-literals", [&] {
                device.showRawDigit(-1, 0xff);
                device.showRawDigit(4, 0xff);
                device.showInteger(-1, 5);
                device.showInteger(4, 5);
                device.showIntegerLiteral(-1);
                device.showIntegerLiteral(10000);
                device.showIntegerLiteral(0x10000, tm1637::RadixHex);
            });

            update("protected-start", [&] { device.start(); });
            update("protected-stop", [&] { device.stop(); });
            update("protected-break", [&] { device.br(); });
            update("protected-write-byte", [&] { device.writeByte(0xa5); });

            fake_wiringpi::reset();
            fake_wiringpi::failAfterWrites(5);
            bool failed = false;
            try {
                device.showRawDigits(1, 2, 3, 4);
            } catch (const std::runtime_error&) {
                failed = true;
            }
            require(failed, "GPIO failure was not propagated synchronously");
            check("failed-update-prefix");
            // No commands left from the failed update may leak into the next one.
            update("clear-after-failure", [&] { device.clear(); });
            fake_wiringpi::reset();
        }
        check("deinitialize-wiringpi");

        fake_wiringpi::reset();
        {
            tm1637::Device device(clockPin, dataPin, tm1637::GpioWiringPiBCM);
            check("initialize-wiringpi-bcm");
            update("bcm-raw-digits", [&] { device.showRawDigits(1, 2, 3, 4); });
            fake_wiringpi::reset();
        }
        check("deinitialize-wiringpi-bcm");

        if (!record) {
            require(checked == expected.size(), "Unused protocol traces");
            std::cout << checked << " protocol traces match; all GPIO calls are synchronous.\n";
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
