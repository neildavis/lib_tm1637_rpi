#pragma once

#include <memory>
#include <vector>
#include <string>

namespace tm1637 {

    class Device;

    class Sayer {

        public:
            Sayer(std::unique_ptr<Device> &device, const std::string &str);
            virtual ~Sayer() = default;
            Sayer(const Sayer &device) = delete;   // copying is not supported
            Sayer& operator=(const Sayer &device) = delete;   // assignment is not supported

            void begin();
            bool next();
            void clear();

        private:
            std::unique_ptr<Device> m_device;
            std::vector<uint8_t> m_digits;
            std::vector<uint8_t>::iterator m_it;
            uint8_t m_data[4];

    };

}