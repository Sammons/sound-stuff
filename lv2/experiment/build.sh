#!/bin/zsh
clang-9 -std=c++2a -Ofast -shared -o experiment.so -fPIC src/plugin.cc -I/usr/include/lv2-c++-tools -lstdc++ -lstdc++fs -lczmq -lzmq -pthread -Wall

# mkdir -p /usr/lib/lv2/lv2-experiment.lv2
# cp experiment.so manifest.ttl experiment.ttl /usr/lib/lv2/lv2-experiment.lv2