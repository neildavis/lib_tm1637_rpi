#!/usr/bin/env bash

g++ -Wall -std=c++11 tm1637_say.cpp -o tm1637_say -lboost_program_options $(pkg-config --libs --cflags libTM1637Pi)