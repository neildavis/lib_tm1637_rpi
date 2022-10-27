cmake_minimum_required(VERSION 3.13)

include(GNUInstallDirs)

set(lib-name TM1637Pi)
set(lib-version 1.0)

# Project
project(${lib-name} VERSION ${lib-version} DESCRIPTION "TM1637 driver for Raspberry Pi")

# We require C++11
set (CMAKE_CXX_STANDARD 11)

# Default to building shared lib (.so) instead of static lib (.a) if not set on cmd line
option(BUILD_SHARED_LIBS "Build shared library (instead of static)" ON)

# Conditional GPIO 'plugin' compilation
set(GPIO_LIB "GPIOD" CACHE STRING "GPIO library: GPIOD/WIRINGPI")

# Library source code location
set(lib_cpp_base ./src)
set(lib_inc_base ./inc)

# Library source core code files
set(lib_core_src_files
    "${lib_cpp_base}/tm1637.cpp"
)

# GPIO: libgpiod support source code files
set(gpio_lib_gpiod_files
    "${lib_cpp_base}/gpioGPIOD.cpp"
)

# GPIO: wiringPi support source code files
set(gpio_lib_wiringpi_files
    "${lib_cpp_base}/gpioWiringPi.cpp"
)

# Library target
add_library(${lib-name} ${lib_core_src_files})

# Include the requested GPIO library specific source code files
if(${GPIO_LIB} STREQUAL "GPIOD")
    target_sources(${lib-name} PUBLIC ${gpio_lib_gpiod_files})
    add_compile_definitions(USE_GPIOD)
elseif (${GPIO_LIB} STREQUAL "WIRINGPI")
    target_sources(${lib-name} PUBLIC ${gpio_lib_wiringpi_files})
    add_compile_definitions(USE_WIRINGPI)
endif()


# Include dirs
target_include_directories(${lib-name} PRIVATE ${lib_inc_base})
target_include_directories(${lib-name} PRIVATE ${lib_cpp_base})
# Public header for the library (installed to sys include)
set_target_properties(${lib-name} PROPERTIES PUBLIC_HEADER ${lib_inc_base}/tm1637.h)
set_target_properties(${lib-name} PROPERTIES VERSION ${PROJECT_VERSION})
# SOVERSION
set_target_properties(${lib-name} PROPERTIES SOVERSION 1)

# Installation of lib & headers
install(TARGETS ${lib-name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# pkg-config support
configure_file(pkg-config/lib${lib-name}.pc.in lib${lib-name}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/lib${lib-name}.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
