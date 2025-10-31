#include "gpioGPIODv2.h"

#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <stdexcept>
#include <vector>

using namespace tm1637;

const char *kGPIODLineConsumer2 = "libTM1637Pi";

/**
 * @brief Direction settings.
 */
enum gpiod_line_direction {
	GPIOD_LINE_DIRECTION_AS_IS = 1,
	/**< Request the line(s), but don't change direction. */
	GPIOD_LINE_DIRECTION_INPUT,
	/**< Direction is input - for reading the value of an externally driven
	 *   GPIO line. */
	GPIOD_LINE_DIRECTION_OUTPUT,
	/**< Direction is output - for driving the GPIO line. */
};

/**
 * @brief Logical line state.
 */
enum gpiod_line_value {
	GPIOD_LINE_VALUE_ERROR = -1,
	/**< Returned to indicate an error when reading the value. */
	GPIOD_LINE_VALUE_INACTIVE = 0,
	/**< Line is logically inactive. */
	GPIOD_LINE_VALUE_ACTIVE = 1,
	/**< Line is logically active. */
};

GPIODv2::GPIODv2(int pinClk, int pinData, void *libHandle) :
    m_pinClk(pinClk),
    m_pinData(pinData),
    m_libHandle(libHandle) {
}


void GPIODv2::initialize() {
    dynLoadGpiodLib();

    std::vector<std::string> chip_paths = {
        "/dev/gpiochip4",   // For RPi5 - try first
        "/dev/gpiochip0"    // Earlier RPi
    };

    gpiod_chip *chip = NULL;
    for (std::vector<std::string>::iterator it = chip_paths.begin();
        it != chip_paths.end(); it++) {
        chip = m_gpiod_chip_open(it->c_str());
        if (chip) {
            break;
        }
    }
    // Get the chip
    if (!chip) {
        throw std::runtime_error("ERROR: Unable to open any known gpiod chip");
    }
    // Create a line settings and set to OUTPUT mode and drive LOW
    gpiod_line_settings* lineSettings = m_gpiod_line_settings_new();
    m_gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    m_gpiod_line_settings_set_output_value(lineSettings, GPIOD_LINE_VALUE_INACTIVE);
    // Create a line config for BOTH CLK & DATA lines
    gpiod_line_config *lineConfig = m_gpiod_line_config_new();
    const unsigned int offsets[] = {m_pinClk, m_pinData};
    m_gpiod_line_config_add_line_settings(lineConfig, offsets, 2, lineSettings);
    // Create a line request config
    gpiod_request_config *requestCfg = m_gpiod_request_config_new();
    m_gpiod_request_config_set_consumer(requestCfg, kGPIODLineConsumer2);
    // Request the lines
    m_lineRequest = m_gpiod_chip_request_lines(chip, requestCfg, lineConfig);

    // Cleanup
    m_gpiod_request_config_free(requestCfg);
    m_gpiod_line_config_free(lineConfig);
    m_gpiod_line_settings_free(lineSettings);
    m_gpiod_chip_close(chip);
}

void GPIODv2::deinitialize() {
    m_gpiod_line_request_release(m_lineRequest);
}

void GPIODv2::setClock(PinDigitalState state) {
    m_gpiod_line_request_set_value(m_lineRequest, m_pinClk, state);
}

void GPIODv2::setData(PinDigitalState state) {
    m_gpiod_line_request_set_value(m_lineRequest, m_pinData, state);
}

void GPIODv2::delayMicroseconds(int usecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}

void GPIODv2::dynLoadGpiodLib() {
    m_gpiod_chip_open = reinterpret_cast<gpiod_chip* (*)(const char*)>(dlsym(m_libHandle, "gpiod_chip_open"));
    if (NULL == m_gpiod_chip_open) { throw std::runtime_error(dlerror()); }
    m_gpiod_chip_close = reinterpret_cast<void (*)(gpiod_chip*)>(dlsym(m_libHandle, "gpiod_chip_close"));
    if (NULL == m_gpiod_chip_close) { throw std::runtime_error(dlerror()); }
    //
    m_gpiod_line_settings_new = reinterpret_cast<gpiod_line_settings* (*)(void)>(dlsym(m_libHandle, "gpiod_line_settings_new"));
    if (NULL == m_gpiod_line_settings_new) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_settings_free = reinterpret_cast<void (*)(gpiod_line_settings*)>(dlsym(m_libHandle, "gpiod_line_settings_free"));
    if (NULL == m_gpiod_line_settings_free) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_settings_set_direction = reinterpret_cast<int (*)(gpiod_line_settings*, int)>(dlsym(m_libHandle, "gpiod_line_settings_set_direction"));
    if (NULL == m_gpiod_line_settings_set_direction) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_settings_set_output_value = reinterpret_cast<int (*)(gpiod_line_settings*, int)>(dlsym(m_libHandle, "gpiod_line_settings_set_output_value"));
    if (NULL == m_gpiod_line_settings_set_output_value) { throw std::runtime_error(dlerror()); }
    //
    m_gpiod_line_config_new = reinterpret_cast<gpiod_line_config* (*)(void)>(dlsym(m_libHandle, "gpiod_line_config_new"));
    if (NULL == m_gpiod_line_config_new) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_config_free = reinterpret_cast<void (*)(gpiod_line_config*)>(dlsym(m_libHandle, "gpiod_line_config_free"));
    if (NULL == m_gpiod_line_config_free) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_config_add_line_settings = reinterpret_cast<int (*)(gpiod_line_config*, const unsigned int*, size_t, gpiod_line_settings*)>(dlsym(m_libHandle, "gpiod_line_config_add_line_settings"));
    if (NULL == m_gpiod_line_config_add_line_settings) { throw std::runtime_error(dlerror()); }
    //
    m_gpiod_request_config_new = reinterpret_cast<gpiod_request_config* (*)(void)>(dlsym(m_libHandle, "gpiod_request_config_new"));
    if (NULL == m_gpiod_request_config_new) { throw std::runtime_error(dlerror()); }
    m_gpiod_request_config_free = reinterpret_cast<void (*)(gpiod_request_config*)>(dlsym(m_libHandle, "gpiod_request_config_free"));
    if (NULL == m_gpiod_request_config_free) { throw std::runtime_error(dlerror()); }
    m_gpiod_request_config_set_consumer = reinterpret_cast<void (*)(gpiod_request_config*, const char*)>(dlsym(m_libHandle, "gpiod_request_config_set_consumer"));
    if (NULL == m_gpiod_request_config_set_consumer) { throw std::runtime_error(dlerror()); }
    //
    m_gpiod_chip_request_lines = reinterpret_cast<gpiod_line_request* (*)(gpiod_chip*, gpiod_request_config*,gpiod_line_config* )>(dlsym(m_libHandle, "gpiod_chip_request_lines"));
    if (NULL == m_gpiod_chip_request_lines) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_request_release = reinterpret_cast<void (*)(gpiod_line_request*)>(dlsym(m_libHandle, "gpiod_line_request_release"));
    if (NULL == m_gpiod_line_request_release) { throw std::runtime_error(dlerror()); }
    m_gpiod_line_request_set_value = reinterpret_cast<int (*)(gpiod_line_request*, unsigned int, int)>(dlsym(m_libHandle, "gpiod_line_request_set_value"));
    if (NULL == m_gpiod_line_request_set_value) { throw std::runtime_error(dlerror()); }
}
