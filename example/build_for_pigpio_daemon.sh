#!/bin/bash

g++ -DGPIO_LIB=GpioPigpioDaemon -Wall -std=c++11 tm1637_example.cpp -o tm1637_example $(pkg-config --libs --cflags libTM1637Pi)