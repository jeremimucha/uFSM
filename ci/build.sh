#!/bin/bash -e

conan install . -if ./build
cmake -G Ninja -S /repo -B ./build
cmake --build ./build
