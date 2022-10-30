#!/bin/bash

g++ -DGPIO_WIRINGPI -Wall -std=c++11 tm1637_example.cpp -o tm1637_example $(pkg-config --libs --cflags libTM1637Pi)