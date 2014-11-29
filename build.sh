#!/usr/bin/sh

gcc -std=c11 -o bytecode_generator bytecode_generator.c
gcc -std=c11 -o stackmachine stackmachine.c
