#!/bin/sh

set -xe

mkdir -p ./build/
mkdir -p ./wasm/

CFLAGS="-O3 -Wall -Wextra -g -pedantic `pkg-config --libs raylib`"
CLIBS="`pkg-config --libs raylib` -lm"

clang $CFLAGS -o ./build/balls ./src/balls.c $CLIBS
clang --target=wasm32 -I./include/ --no-standard-libraries -Wl,--export-table -Wl,--no-entry -Wl,--allow-undefined -Wl,--export=main -Wl,--export=__head_base -Wl,--allow-undefined -o ./wasm/balls.wasm ./src/balls.c -DPLATFORM_WEB

