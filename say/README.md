# tm1637_say

A simple utility using [lib_tm1637_rpi](../README.md) to scroll some text on a [TM1637](https://datasheetspdf.com/pdf-file/788613/TitanMicro/TM1637/1) display.

## Building

Ensure you have [built & installed libTM1637RPi](../README.md#building--installing) before building `tm1637_say`.

Use the [build.sh](./build.sh) script to build the utility.

```sh
./build.sh
```

## Running

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