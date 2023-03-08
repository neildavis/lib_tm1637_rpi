# tm1637_say

A simple utility using [lib_tm1637_rpi](../README.md) to scroll some text on a [TM1637](https://datasheetspdf.com/pdf-file/788613/TitanMicro/TM1637/1) display.

## Building

Ensure you have [built & installed libTM1637RPi](../README.md#building--installing) before building `tm1637_say`.

Use the [build.sh](./build.sh) script to build the utility. By default, the utility will use the `GPIOD` library. This can be overridden by setting the `GPIO_LIB` environment variable to one of the `GPIOLib` enum values defined in [tm1637.h](../inc/tm1637.h#L34)

 e.g. to build using `wiringPi` (with BCM pin numbering):

```sh
GPIO_LIB="GpioWiringPiBCM" ./build.sh
```

## Running

Running `tm1637_say` without any arguments, or with `--help` will display help & usage information:

```sh
$ tm1637_say 
tm1637_say utility - © 2023 Neil Davis
See LICENSE at https://github.com/neildavis/lib_tm1637_rpi/blob/main/LICENSE

Usage: tm1637_say [OPTIONS] <message>

Available OPTIONS:
  --help                         Show usage information
  -c [ --scl ] arg (=3)          GPIO pin to use for clock
  -d [ --sda ] arg (=2)          GPIO pin to use for data
  -t [ --delay-time ] arg (=250) Delay time between characters (ms)
```

e.g. the following invocation uses pins 23 & 24 for `SCL` & `SDA` respectively, and displays a message faster than the default:

```sh
tm1637_say -c 23 -d 24 -t 100 "HELLO WORLD"
```