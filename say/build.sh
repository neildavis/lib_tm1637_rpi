#!/usr/bin/env bash

if [[ -z "${GPIO_LIB}" ]]; then
    export GPIO_LIB="GpioGPIOD"
fi
echo "Building using GPIO library ${GPIO_LIB} - override by setting GPIO_LIB"
g++ -DGPIO_LIB="${GPIO_LIB}" -Wall -std=c++11 tm1637_say.cpp -o tm1637_say -lboost_program_options $(pkg-config --libs --cflags libTM1637Pi)