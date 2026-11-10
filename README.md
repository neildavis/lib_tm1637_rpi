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
(see [instructions](#install-supported-gpio-libraries) below), but your program **does not** have to link
to any GPIO library at build time. The GPIO library is dynamically loaded at runtime using
[`dlopen()`](https://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html).

## Features

* Show any combination of segments on up to four digits controlled by the TM1637.
* Independent control over the 'colon' character.
* Independent control over the display brightness.
* APIs to show numerical literals in both decimal (e.g. `1234`) & hexadecimal (e.g. `0xabcd`).
* Optional worker-thread execution with FIFO updates and queued display pauses.

## Installation

With [releases](https://github.com/neildavis/lib_tm1637_rpi/releases) from **v1.4.0** onwards
I am making available prebuilt Debian (`.deb`) packages for both
32-bit (`armhf`) & 64-bit (`arm64`/`aarch64`) versions of Raspberry Pi OS
(Debian versions 12 '_Bookworm_' / 13 '_Trixie_').

Download them from the [releases]([releases](https://github.com/neildavis/lib_tm1637_rpi/releases)) and install using `dpkg -i`: e.g.

```sh
# 64-bit
sudo dpkpg -i  libtm1637pi_1.5_arm64.deb`
```

OR

```sh
# 32-bit
sudo dpkpg -i  libtm1637pi_1.5_armhf.deb`
```

Alternatively, you can follow the instructions in this README to
[Build & Install From Source](#build--install-from-source)


## Install Supported GPIO libraries 

### libgpiod

If you wish to use `libgpiod` for GPIO access, you will need to install the `gpiod` package:

```sh
sudo apt install gpiod
```

The library supports both SO versions of `libgpiod3` - as included in Debian 13 'Trixie'- 
and the older `libgpiod2` - as included in previous OS versions.

Although these versions are neither source nor binary compatible (API & ABI change),
this library will identify and dynamically load the installed version and call the appropriate API.

### pigpio / pigpiod

#### \*\* IMPORTANT - Raspberry Pi 5 \*\*

Due to hardware architecture changes in the Raspberry Pi 5, ***`pigpio`*** 
**does not currently work on the Raspberry Pi 5**. See / track
[these](https://github.com/joan2937/pigpio/issues/586)
[issues](https://github.com/joan2937/pigpio/issues/589)
for more details.


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

`wiringPi` was [removed from v11 ('Bullseye')](https://github.com/RPi-Distro/repo/issues/214#issuecomment-1016542851) of Raspberry Pi OS packages. If you wish to use `wiringPi` for GPIO access you will need to build and/or install it manually by following the instructions in [WiringPi's README](https://github.com/WiringPi/WiringPi/blob/master/README.md#installing)

## Build & Install From Source

### Prerequisites

You will need [cmake](https://cmake.org/) (v3.9 or higher) and the necessary C++ development tools
installed in order to build and install the library.
In addition, the `say` utility requires the development files for the
[`program_options`](https://www.boost.org/doc/libs/latest/doc/html/program_options.html)
component from Boost (v1.70 or higher). These requirements have been satisfied by apt packages in Debian repos since v11 ('Bullseye'):

```sh
sudo apt install build-essential cmake libboost-program-options-dev
```

### Configure the build

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

***OR*** if you prefer to configure the library to build as a [static library](https://en.wikipedia.org/wiki/Static_library) (.a) then you must declare `BUILD_SHARED_LIBS` as `OFF` when calling `cmake`. e.g.

```sh
cmake -DBUILD_SHARED_LIBS=OFF -G"Unix Makefiles" ..
```

### **IMPORTANT**: Changing configuration

If you wish to change between the configuration options above, be sure to delete the `CMakeCache.txt` file **before** calling `cmake` again. It's also wise to perform a '`make clean`' before rebuilding:

```sh
make clean
rm CMakeCache.txt
```

### Building & Install

After [configuring](#configure-the-build) the build, you can build and install the library by simply invoking
[`make`](https://www.gnu.org/software/make/) from the `build` directory:

```sh
sudo make install
sudo ldconfig
```

## Usage

### pkg-config

For ease of use the library installs with [`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/) support.
This means you can automatically find all the required compiler command line options for using the library by issuing the command:

```sh
pkg-config --libs --cflags libTM1637Pi
```

An example of how to compile & link your program against the library:

```sh
g++ -Wall -std=c++14 myprogram.cpp -o myprogram $(pkg-config --libs --cflags libTM1637Pi)
```

### API

The public API is included in the public header file [`tm1637.h`](./inc/tm1637.h)
which will be installed into the system default includes directory (e.g. `/usr/local/include`).

Existing constructors execute synchronously on the calling thread. Each display
method finishes transmitting its update before returning. `wait(duration)` then
pauses that thread for the requested number of milliseconds.

To schedule updates on a worker thread, select `ExecutionMode::Worker`:

```cpp
tm1637::DeviceOptions options;
options.execution = tm1637::ExecutionMode::Worker;
tm1637::Device display(23, 24, tm1637::GpioGPIOD, options);

display.showIntegerLiteral(1234);
display.wait(std::chrono::milliseconds{500});
display.showIntegerLiteral(5678);
display.wait(std::chrono::milliseconds{500});
display.clear();
display.flush();
```

Worker mode appends complete updates and pauses to an unbounded FIFO queue. It
preserves every accepted request and never preempts an executing update. Submitted
values are copied into commands, so input arrays need not remain alive. Queue
allocation failures are reported to the submitting caller; previously accepted
work remains intact. Callers should avoid producing an unlimited backlog.

`wait(std::chrono::milliseconds)` schedules a pause after preceding updates in worker
mode; zero and negative durations do nothing. Pauses use standard C++ sleeping,
separately from the backend's GPIO protocol delays. Scheduling may extend the
requested interval; this is not a real-time timing guarantee.

`flush()` blocks until all work accepted before that call has finished, including
an executing update and queued pauses. It returns immediately in synchronous mode.
With a worker-backed device, `Sayer::finished()` means all characters have been
submitted; call the device's `flush()` to establish completion.

Construction waits for initialization to succeed. Backend creation, initialization,
GPIO calls, protocol waits, deinitialization, and backend destruction all execute
on the worker in worker mode. Normal destruction drains accepted updates and pauses
before cleanup and joining the worker; long queued pauses also delay destruction.
There is no cancellation or timed shutdown. A worker failure discards remaining
work and is rethrown by `flush()` and subsequent submissions; recreate the device to
recover. Destruction is non-throwing and does not report cleanup errors.

Externally serialize calls to each `Device`, including destruction. Worker mode
does not make concurrent public API calls safe and does not coordinate separate
devices or processes sharing GPIO pins. Use one active owner per physical display.
Protected protocol helpers follow the selected execution mode; subclasses must
only access `m_gpio` directly in synchronous mode.

Version **2.0** uses **SOVERSION 2** (`libTM1637Pi.so.2`). Existing public source
usage remains supported, but applications must rebuild and relink to upgrade from
version 1; binary compatibility with `libTM1637Pi.so.1` is not retained.

### Hardware-free tests

The optional tests use a recording backend instead of accessing GPIO hardware.
They compare original protocol traces in both execution modes and check FIFO
ordering, argument ownership, queued pauses, completion, allocation and backend
failures, partial initialization cleanup, and thread ownership of the lifecycle.

```sh
cmake -S . -B build/tests -DBUILD_TESTING=ON
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

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

The utility always uses synchronous execution and `Device::wait()` for character
intervals. It completes scrolling, clears the display, and releases GPIO before
normal exit. Concurrent utility instances accessing the same pins remain unsupported.

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
