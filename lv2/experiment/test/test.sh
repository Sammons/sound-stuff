#!/bin/bash

clang-9 -std=c++2a resonator.cc -lstdc++ -lsndfile -lfftw3 -lm -o resonator
./resonator