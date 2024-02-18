#!/bin/bash

clang --target=wasm32 -I./include -I../build/_deps/raylib-src/src/ -I../src --no-standard-libraries -Wl,--export-table -Wl,--no-entry -Wl,--allow-undefined -Wl,--export=main -o wasm/particle_simulator.wasm particle_simulator.c ../src/*.c -DSPH_NO_STDIO
