#!/bin/bash

set -e

BUILD_DIR="build_emulator"

if [ "$1" == "--clean" ] || [ "$1" == "-c" ]; then
    echo "cleaning..."
    rm -rf "$BUILD_DIR"
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "creating build dir & running cmake"
    mkdir "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DEMULATOR=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
    cd ..
fi

echo "compiling code..."
make -C "$BUILD_DIR" -j$(sysctl -n hw.ncpu)

echo "starting ..."
./$BUILD_DIR/emulator
