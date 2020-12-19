#!/bin/bash

g++ -std=c++1y -I/usr/local/include/ ./example.cc -lczmq -lzmq -lpthread -o example