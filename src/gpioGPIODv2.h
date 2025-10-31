#pragma once

#include "gpio.h"
#include <cstddef>

struct gpiod_chip;
struct gpiod_line_settings;
struct gpiod_line_request;
struct gpiod_line_config;
struct gpiod_request_config;

namespace tm1637 {
    
    class GPIODv2 : public MGPIO {
    public:
        GPIODv2(int pinClk, int pinData, void *libHandle);
        // API
        virtual void initialize() override;
        virtual void deinitialize() override;
        virtual void setClock(PinDigitalState state) override;
        virtual void setData(PinDigitalState state) override;
        virtual void delayMicroseconds(int usecs) override;
    private:
        void dynLoadGpiodLib();
    private:
        // These are func pointers we assign from the symbols in libgpiod.so.3
        gpiod_chip* (*m_gpiod_chip_open)(const char *name);
        void (*m_gpiod_chip_close)(gpiod_chip *chip);
        //
        gpiod_line_settings* (*m_gpiod_line_settings_new)(void);
        void (*m_gpiod_line_settings_free)(gpiod_line_settings *settings);
        int (*m_gpiod_line_settings_set_direction)(gpiod_line_settings *settings, int  direction);
        int (*m_gpiod_line_settings_set_output_value)(gpiod_line_settings *settings, int  value);
        //
        gpiod_line_config* (*m_gpiod_line_config_new)(void);
        void (*m_gpiod_line_config_free)(gpiod_line_config *config);
        int (*m_gpiod_line_config_add_line_settings)(gpiod_line_config *config,
            const unsigned int *offsets, size_t num_offsets, gpiod_line_settings *settings);
        //
        gpiod_request_config* (*m_gpiod_request_config_new)(void);
        void (*m_gpiod_request_config_free)(gpiod_request_config *config);
        void (*m_gpiod_request_config_set_consumer)(gpiod_request_config *config, const char *consumer);
        //
        gpiod_line_request* (*m_gpiod_chip_request_lines)(gpiod_chip *chip,
            gpiod_request_config *req_cfg, gpiod_line_config *line_cfg);
        void (*m_gpiod_line_request_release)(gpiod_line_request *request);
        int (*m_gpiod_line_request_set_value)(gpiod_line_request *request,
				 unsigned int offset,
				 int value);


    private:
        unsigned int m_pinClk;
        unsigned int m_pinData;
        void *m_libHandle;
        gpiod_line_request *m_lineRequest;
   };
}
