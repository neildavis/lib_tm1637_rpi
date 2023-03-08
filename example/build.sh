#!/usr/bin/env bash

if [[ -z "${GPIO_LIB}" ]]; then
    export GPIO_LIB="GpioGPIOD"
fi
echo "Building using GPIO library ${GPIO_LIB} - override by setting GPIO_LIB"
g++ -DGPIO_LIB=GpioGPIOD -Wall -std=c++11 tm1637_example.cpp -o tm1637_example $(pkg-config --libs --cflags libTM1637Pi)