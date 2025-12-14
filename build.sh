#!/bin/bash
mkdir -p build
cd build
cmake ..
make flash clean_all
cd ..
picocom --lower-dtr -b 115200 /dev/ttyUSB0
