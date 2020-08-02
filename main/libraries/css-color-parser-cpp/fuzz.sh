#!/usr/bin/env sh

make fuzz
afl-fuzz -i input -o output -- ./fuzz
