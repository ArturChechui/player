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

# Create build directory
mkdir -p tests/unit/build
cd tests/unit/build

# Configure
cmake ..

# Build
cmake --build . -j$(nproc)

# Run
ctest --output-on-failure --verbose

# Or run directly
./host_tests
