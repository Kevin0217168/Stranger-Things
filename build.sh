#!/bin/bash
make all info flash clean && picocom --lower-dtr -b 115200 /dev/ttyUSB0
