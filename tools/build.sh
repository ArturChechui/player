#!/bin/bash

# 1) load ESP-IDF environment
adf

# 2) set target once
idf.py set-target esp32s3

# 3) build
idf.py build

# 4) flash + monitor:
# idf.py -p /dev/ttyACM0 flash monitor

# 5) other useful commands:
# idf.py menuconfig
# idf.py fullclean
# idf.py monitor
