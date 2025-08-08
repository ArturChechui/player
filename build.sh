#!/bin/bash

cmake -S . -B build -DCMAKE_CXX_STANDARD=23 -DMY_DEFINE=1 && cmake --build build
