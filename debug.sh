#!/bin/bash

# Clear terminal
clear

# Navigate to bin/ and build the project
cd bin
make clean
make

clear

# Start gdb with the compiled binary
gdb -q ./CmOS

# Chose the line at which to break
# break fileSystem.c:94

# Run the program
# run

