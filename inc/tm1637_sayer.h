#pragma once

#include <memory>
#include <vector>
#include <string>

namespace tm1637 {

    class Device;

    class Sayer {

        public:
            Sayer(const std::shared_ptr<Device> &device);
            virtual ~Sayer() = default;
            Sayer(const Sayer &device) = delete;   // copying is not supported
            Sayer& operator=(const Sayer &device) = delete;   // assignment is not supported

            void begin(const std::string &str);
            bool next();
            bool finished() const;
            void reset();   // returns to beginning of current string
            void clear();   // clears the display and current string 
        
        private:
            void setDigitsFromStr(const std::string &str);

        private:
            std::shared_ptr<Device> m_device;
            std::vector<uint8_t> m_digits;
            std::vector<uint8_t>::iterator m_it;
            uint8_t m_data[4];

    };

}