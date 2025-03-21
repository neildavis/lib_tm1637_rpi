# lib_tm1637_rpi

A library for using the [TM1637](https://datasheetspdf.com/pdf-file/788613/TitanMicro/TM1637/1)
LED driver control IC to drive a 4 x 7-segment display from Raspberry Pi OS (Raspbian). Includes a small [`say`](./say/) utility to scroll some text.

This library does not access the GPIO pins itself. Instead, it has been designed to allow the use of
different 'backend' GPIO libraries through the use of
[polymorphism](https://en.wikipedia.org/wiki/Polymorphism_(computer_science)) and
[dynamic loading](https://en.wikipedia.org/wiki/Dynamic_loading)
to provide a ***runtime choice*** of GPIO library.
The currently supported GPIO libraries include:

* [`libgpiod`](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/)
* [`pigpio`](https://abyz.me.uk/rpi/pigpio/) (Both 'library' and '[`pigpiod`](https://abyz.me.uk/rpi/pigpio/pigpiod.html)' interfaces)
* [`wiringPi`](https://github.com/WiringPi/WiringPi)

By default, `libgpiod` will be used.

You must ensure you have installed the GPIO library that you wish to use
(see [Prerequisites](#prerequisites) below), but your program **does not** have to link
to any GPIO library at build time. The GPIO library is dynamically loaded at runtime using
[`dlopen()`](https://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html).

## Features

* Show any combination of segments on up to four digits controlled by the TM1637.
* Independent control over the 'colon' character.
* Independent control over the display brightness.
* APIs to show numerical literals in both decimal (e.g. `1234`) & hexadecimal (e.g. `0xabcd`).

## Prerequisites

### Building

You will need [cmake](https://cmake.org/) v3.9 or higher and the necessary C++ development tools
installed in order to build and install the library.
In addition, the `say` utility requires the development files for the `program_options`
component from Boost:

```sh
sudo apt install build-essential cmake libboost-program-options-dev
```

### libgpiod

If you wish to use `libgpiod` for GPIO access, you will need to install the `gpiod` package:

```sh
sudo apt install gpiod
```

### pigpio / pigpiod

If you wish to use the `pigpio` library and/or `pigpiod` for GPIO access, you may need to install the `pigpiod` package
(although it may already be included by default in your OS image):

```sh
sudo apt install pigpiod
```

When using the `pigpio` 'library interface' your app will need to run as the '`root`' user (or under '`sudo`')
to have permission to access the GPIO pins. You must also ensure that `pigpiod` is **NOT** currently running.

When using the `pigpiod` daemon, the library will connect to the daemon on `localhost` by default.
If you wish to connect to a daemon on a different interface (or machine) you can do this by setting
the `PIGPIO_ADDR` and `PIGPIO_PORT` environment variables as described in the documentation for
[`pigpio_start()`](https://abyz.me.uk/rpi/pigpio/pdif2.html#pigpio_start)

### WiringPi

`wiringPi` was [removed from v11 ('Bullseye')](https://github.com/RPi-Distro/repo/issues/214#issuecomment-1016542851) of Raspberry Pi OS packages. If you wish to use `wiringPi` for GPIO access you will need to build & install it manually by following the
[instructions](https://github.com/WiringPi/WiringPi/blob/master/INSTALL).
Create a new directory somewhere, clone the
[source](https://github.com/WiringPi/WiringPi.git) and build & install:

```sh
git clone --depth 1 https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build
ldconfig
```

## Building & Installing

### Build Configuration

From this repo's root directory, create and change to a new `build` directory:

```sh
mkdir build
cd build
```

#### Shared vs Static Library

By default the library will be configured to build as a [shared library](https://en.wikipedia.org//wiki/Library_(computing)#Shared_libraries) (.so) which is the recommended usage:

```sh
cmake -G"Unix Makefiles" ..
```

If you prefer to configure the library to build as a [static library](https://en.wikipedia.org/wiki/Static_library) (.a) then you must declare `BUILD_SHARED_LIBS` as `OFF` when calling `cmake`. e.g.

```sh
cmake -DBUILD_SHARED_LIBS=OFF -G"Unix Makefiles" ..
```

### **IMPORTANT**: Changing configuration

If you wish to change the configuration options above, be sure to delete the `CMakeCache.txt` file **before** calling `cmake` again. It's also wise to perform a '`make clean`' before rebuilding:

```sh
make clean
rm CMakeCache.txt
```

### Building & Install

After configuring the build, you can build and install the library by simply invoking
[`make`](https://www.gnu.org/software/make/) from the `build` directory:

```sh
sudo make install
sudo ldconfig
```

You may or may not need to use the `sudo` prefix this command depending on your user permissions:

## Usage

### pkg-config

For ease of use the library installs with [`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/) support.
This means you can automatically find all the required compiler command line options for using the library by issuing the command:

```sh
pkg-config --libs --cflags libTM1637Pi
```

An example of how to compile & link your program against the library:

```sh
g++ -Wall -std=c++11 myprogram.cpp -o myprogram $(pkg-config --libs --cflags libTM1637Pi)
```

### Compiling

The library makes use of [C++11](https://en.cppreference.com/w/cpp/11) features, so you will need to include
the relevant flag when invoking your compiler. e.g. for (modern versions of) GCC (`g++`) you would use '`-std=c++11`'

The library's public headers are installed into the system default directory (e.g. `/usr/local/include`).
Add the following to your `CPPFLAGS` when invoking the compiler, **ONLY** if you are not using `pkg-config`:

```sh
-I/usr/local/include
```

### Linking

The library is installed into the system default directory (e.g. `/usr/local/lib`).
Add the following options when invoking your compiler, **ONLY** if you are not using `pkg-config`:

```sh
-L/usr/local/lib -lTM1637Pi -ldl
```

### API

The public API is included in the public header file [`tm1637.h`](./inc/tm1637.h)
which will be installed into the system default includes directory (e.g. `/usr/local/include`).

### Example

See the [example](./example/tm1637_example.cpp) code in the `example` directory for a demonstration of the library's features. The example can be built and run with the following commands from the top level dir of this repo:

```sh
cd example
./build_for_gpiod.sh
# or ./build_for_wiringpi.sh 
# or ./build_for_pigpio_interface.sh 
# or ./build_for_pigpio_daemon.sh 
./tm1637_example 
```

## tm1637_say utility

A simple utility using `lib_tm1637_rpi` to scroll some text on a [TM1637](https://datasheetspdf.com/pdf-file/788613/TitanMicro/TM1637/1) display is included as part of the build. The cmake `install` action will also install it into the default binary installation directory which should be already included in your `$PATH`

### tm1637_say usage

Running `tm1637_say` without any arguments, or with `--help` will display help & usage information:

```none
$ tm1637_say 
tm1637_say utility - © 2023 Neil Davis
See LICENSE at https://github.com/neildavis/lib_tm1637_rpi/blob/main/LICENSE

Usage: tm1637_say [OPTIONS] <message>

Available OPTIONS:
  -h [ --help ]                      Show usage information
  -c [ --scl ] arg (=3)              GPIO pin to use for clock
  -d [ --sda ] arg (=2)              GPIO pin to use for data
  -g [ --gpio-lib ] arg (=GpioGPIOD) GPIO library
  -t [ --delay-time ] arg (=250)     Delay time between characters (ms)
  -n [ --count ] arg (=1)            Repeat the message <arg> number of times
  -v [ --verbose ] [=arg(=1)] (=0)   Enable/Disable verbose output
```

e.g. the following invocation uses the wiringPi library with BCM pin numbering, pins 23 & 24 for `SCL` & `SDA` respectively, and displays a message faster than the default three times over :

```sh
tm1637_say -c 23 -d 24 -g GpioWiringPiBCM -t 100 -n 3 "HELLO WORLD"
```
