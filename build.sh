#!/bin/bash
make clean all info flash && picocom --lower-dtr -p 2 -b 115200 /dev/ttyUSB0
