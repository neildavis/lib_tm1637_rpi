# lib_tm1637_rpi
A library for using the [TM1637](https://datasheetspdf.com/pdf-file/788613/TitanMicro/TM1637/1) LED driver control IC to drive a 4 x 7-segment display from Raspberry Pi OS (Raspbian). 

Currently, the library uses [wiringPi](https://github.com/WiringPi/WiringPi) for access to the GPIO pins. However it has been designed to allow use of other 'backend' GPIO libraries through ['polymorphism'](https://en.wikipedia.org/wiki/Polymorphism_(computer_science)). Future updates may make use of ['dynamic loading'](https://en.wikipedia.org/wiki/Dynamic_loading) to provide runtime choice of other GPIO libraries (e.g. [pigpio](https://abyz.me.uk/rpi/pigpio/).)

## Features
* Show any combination of segments on up to four digits controlled by the TM1637.
* Independent control over the 'colon' character.
* Independent control over the display brightness.
* APIs to show numerical literals in both decimal & hexadecimal.

## Building & Installing
You will need [cmake](https://cmake.org/) v3.9 or higher in order to build the library.

### Build
From this repo's root directory, create and change to a new `build` directory:
```sh
mkdir build
cd build
```
By default the library will be configured to build as a [shared library](https://en.wikipedia.org//wiki/Library_(computing)#Shared_libraries) (.so) which is the recommended usage:
```sh
cmake -G"Unix Makefiles" ..
```
If you prefer to configure the library to build as a [static library](https://en.wikipedia.org/wiki/Static_library) (.a) then use this command instead:
```sh
cmake -DBUILD_SHARED_LIBS=OFF -G"Unix Makefiles" ..
```
Then to build the library, simply invoke [`make`](https://www.gnu.org/software/make/):
```sh
make
```

### Install
After building, simply invoke `make` again to install. You may need to prefix this command with `sudo` if you don't have the correct permissions:
```sh
make install
```

## Usage
### pkg-config
For ease of use the library installs with [`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/) support. This means you can automatically find all the required compiler command line options by issuing the command:
```sh
pkg-config --libs --cflags libTM1637Pi
```
An example of how to compile your program against the library:
```sh
g++ -Wall -std=c++11 myprogram.cpp -o myprogram $(pkg-config --libs --cflags libTM1637Pi)
```

### Compiling
The library makes use of [C++11](https://en.cppreference.com/w/cpp/11) features, so you will need to include the relevant flag when invoking your compiler. e.g. for (modern versions of) GCC (`g++`) you would use '`-std=c++11`'

The library's public headers are installed into the system default directory (e.g. `/usr/local/include`)
so add the following to your `CPPFLAGS` when invoking the compiler, ONLY if you are not using `pkg-config`:
```
-I/usr/local/include
```
### Linking
The library is installed into the system default directory (e.g. `/usr/local/lib`). Add the following options when involing your compiler, ONLY if you are not using `pkg-config`: :
```
-L/usr/local/lib -lTM1637Pi -lwiringPi
```

### API
The public API is included in the public header file [`tm1637.h`](./inc/tm1637.h) which should be installed into the system default includes directory.

### Example
See the [example](./example/tm1637_example.cpp) code in the `example` directory for a demonstration of the library's features. The example can be built and run with the following commands from the top level dir of this repo:
```sh
cd example
./build.sh
./tm1637_example 
```