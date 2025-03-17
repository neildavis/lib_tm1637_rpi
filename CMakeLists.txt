cmake_minimum_required(VERSION 3.9)


set(lib-name TM1637Pi)
set(lib-version 1.1)
set(say-name tm1637_say)

# Project
project(${lib-name} VERSION ${lib-version} DESCRIPTION "TM1637 driver for Raspberry Pi")

include(GNUInstallDirs)

# We require C++11
set (CMAKE_CXX_STANDARD 11)

# Default to building shared lib (.so) instead of static lib (.a) if not set on cmd line
option(BUILD_SHARED_LIBS "Build shared library (instead of static)" ON)

# Library source code location
set(lib_cpp_base ./src)
set(lib_inc_base ./inc)

# Library source core code files
file(GLOB_RECURSE lib_src_files "${lib_cpp_base}/*.cpp")

# # Public header files
file(GLOB_RECURSE public_inc_files "${lib_inc_base}/*.h")

# Library target
add_library(${lib-name} ${lib_src_files})

# Include dirs
target_include_directories(${lib-name} PUBLIC ${lib_inc_base})
target_include_directories(${lib-name} PRIVATE ${lib_cpp_base})
# Public header for the library (installed to sys include)
set_target_properties(${lib-name} PROPERTIES PUBLIC_HEADER "${public_inc_files}")
set_target_properties(${lib-name} PROPERTIES VERSION ${PROJECT_VERSION})
# SOVERSION
set_target_properties(${lib-name} PROPERTIES SOVERSION 1)

# Say executable source code location
set(say_cpp_base ./say)
# Say source core code files
file(GLOB_RECURSE say_src_files "${say_cpp_base}/*.cpp")
# Say executable target
add_executable(${say-name} ${say_src_files})
# find required libs
find_package(Boost COMPONENTS program_options REQUIRED)
# Link say executable with library and Boost boost_program_options
target_link_libraries(${say-name} ${lib-name} dl Boost::program_options)


# Installation of lib & headers
install(TARGETS ${lib-name} ${say-name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# pkg-config support
configure_file(pkg-config/lib${lib-name}.pc.in lib${lib-name}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/lib${lib-name}.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
