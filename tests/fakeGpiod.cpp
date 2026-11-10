#include <cerrno>
#include <cstddef>
#include <string>

struct gpiod_line { bool requested = false; unsigned int offset = 0; };
struct gpiod_chip { gpiod_line clock{false, 23}; gpiod_line data{false, 24}; };
struct gpiod_line_settings {};
struct gpiod_line_config {};
struct gpiod_request_config {};
struct gpiod_line_request {};

namespace {
std::string failure;
int resources = 0;
int invalidReleases = 0;

bool failing(const char* operation) {
    if (failure == operation) { errno = EIO; return true; }
    return false;
}

template<class T> T* allocate(const char* operation) {
    if (failing(operation)) { return nullptr; }
    auto result = new T;
    ++resources;
    return result;
}

template<class T> void release(T* pointer) {
    if (!pointer) { ++invalidReleases; return; }
    --resources;
    delete pointer;
}
}

extern "C" {
// Controls are used only before submission or after joining the worker.
void fakeGpiodFailure(const char* operation) { failure = operation; }
int fakeGpiodResources() { return resources; }
int fakeGpiodInvalidReleases() { return invalidReleases; }

gpiod_chip* gpiod_chip_open(const char*) { return allocate<gpiod_chip>("open"); }
gpiod_chip* gpiod_chip_open_by_name(const char*) { return allocate<gpiod_chip>("open"); }
void gpiod_chip_close(gpiod_chip* chip) {
    if (chip && (chip->clock.requested || chip->data.requested)) { ++invalidReleases; }
    release(chip);
}

gpiod_line* gpiod_chip_get_line(gpiod_chip* chip, unsigned int offset) {
    if (failing("line")) { return nullptr; }
    return offset == 23 ? &chip->clock : &chip->data;
}
int gpiod_line_request_output(gpiod_line* line, const char*, int) {
    if (failing(line->offset == 23 ? "v1-clock" : "v1-data")) { return -1; }
    line->requested = true;
    ++resources;
    return 0;
}
void gpiod_line_release(gpiod_line* line) {
    if (!line || !line->requested) { ++invalidReleases; return; }
    line->requested = false;
    --resources;
}
int gpiod_line_set_value(gpiod_line*, int) { return failing("write") ? -1 : 0; }

gpiod_line_settings* gpiod_line_settings_new() {
    return allocate<gpiod_line_settings>("settings");
}
void gpiod_line_settings_free(gpiod_line_settings* settings) { release(settings); }
int gpiod_line_settings_set_direction(gpiod_line_settings*, int) {
    return failing("direction") ? -1 : 0;
}
int gpiod_line_settings_set_output_value(gpiod_line_settings*, int) {
    return failing("output") ? -1 : 0;
}
gpiod_line_config* gpiod_line_config_new() { return allocate<gpiod_line_config>("config"); }
void gpiod_line_config_free(gpiod_line_config* config) { release(config); }
int gpiod_line_config_add_line_settings(gpiod_line_config*, const unsigned int*,
                                      std::size_t, gpiod_line_settings*) {
    return failing("add") ? -1 : 0;
}
gpiod_request_config* gpiod_request_config_new() {
    return allocate<gpiod_request_config>("request-config");
}
void gpiod_request_config_free(gpiod_request_config* config) { release(config); }
void gpiod_request_config_set_consumer(gpiod_request_config*, const char*) {}
gpiod_line_request* gpiod_chip_request_lines(gpiod_chip*, gpiod_request_config*,
                                           gpiod_line_config*) {
    return allocate<gpiod_line_request>("request");
}
void gpiod_line_request_release(gpiod_line_request* request) { release(request); }
int gpiod_line_request_set_value(gpiod_line_request*, unsigned int, int) {
    return failing("write") ? -1 : 0;
}
}
