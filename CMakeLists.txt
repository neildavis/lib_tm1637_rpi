cmake_minimum_required(VERSION 3.9)

include(GNUInstallDirs)

set(lib-name TM1637Pi)
set(lib-version 1.0)

# Project
project(${lib-name} VERSION ${lib-version} DESCRIPTION "TM1637 driver for Raspberry Pi")

# We require C++11
set (CMAKE_CXX_STANDARD 11)

# Default to building shared lib (.so) instead of static lib (.a) if not set on cmd line
option(BUILD_SHARED_LIBS "Build shared library (instead of static)" ON)

# Library source code location
set(lib_cpp_base ./src)
set(lib_inc_base ./inc)

# Library source code files
set(lib_src_files
    "${lib_cpp_base}/tm1637.cpp"
)

# Library target. Use BUILD_SHARED_LIBS to detrmine STATIC vs SHARED
add_library(${lib-name} ${lib_src_files})

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
