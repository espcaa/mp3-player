#!/bin/bash

set -e

BUILD_DIR="build_pico"

# the cmake logic builds for pico only when PICO_SDK_PATH is set and EMULATOR is
# not. without it, cmake would fall through to the SDL2 emulator target.
if [ -z "$PICO_SDK_PATH" ]; then
    echo "error: PICO_SDK_PATH is not set."
    echo "point it at your pico-sdk checkout first, e.g.:"
    echo "  export PICO_SDK_PATH=\$HOME/pico/pico-sdk"
    exit 1
fi

if [ -z "$PICO_EXTRAS_PATH" ]; then
    echo "error: PICO_EXTRAS_PATH is not set (needed for pico_audio_i2s)."
    echo "  git clone https://github.com/raspberrypi/pico-extras"
    echo "  export PICO_EXTRAS_PATH=\$HOME/pico/pico-extras"
    exit 1
fi

# RP2350B target. override via env if your board file differs, e.g.
#   PICO_BOARD=my_board ./build_pico.sh
PICO_PLATFORM="${PICO_PLATFORM:-rp2350}"
PICO_BOARD="${PICO_BOARD:-pico2}"

if [ "$1" == "--clean" ] || [ "$1" == "-c" ]; then
    echo "cleaning..."
    rm -rf "$BUILD_DIR"
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "creating build dir & running cmake"
    echo "  platform=$PICO_PLATFORM board=$PICO_BOARD"
    mkdir "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DPICO_PLATFORM="$PICO_PLATFORM" -DPICO_BOARD="$PICO_BOARD" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
    cd ..
fi

echo "compiling firmware..."
make -C "$BUILD_DIR" -j$(sysctl -n hw.ncpu)

echo ""
echo "done -> $BUILD_DIR/firmware.uf2"
