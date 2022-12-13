#pragma once

#include "gpio.h"
#include <gpiod.h>

namespace tm1637 {
    
    class GPIOD : public MGPIO {
    public:
        GPIOD(int pinClk, int pinData);
        // API
        virtual void initialize() override;
        virtual void deinitialize() override;
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        void dynLoadGpiodLib();
    private:
        // These are func pointers we assign from the symbols in libgpiod.so.2
        gpiod_chip* (*m_gpiod_chip_open_by_name)(const char *name);
        void (*m_gpiod_chip_close)(gpiod_chip *chip);
        gpiod_line* (*m_gpiod_chip_get_line)(gpiod_chip *chip, unsigned int offset);
        void (*m_gpiod_line_release)(gpiod_line *line);
        int (*m_gpiod_line_request_output)(gpiod_line *line, const char *consumer, int default_val);
        int (*m_gpiod_line_set_value)(gpiod_line *line, int value);
    private:
        int m_pinClk;
        int m_pinData;
        void *m_libHandle;
        gpiod_line *m_lineClk;
        gpiod_line *m_lineData;
        gpiod_chip *m_chip;
   };
}
