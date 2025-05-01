#!/bin/bash

# Clear terminal
clear

# Navigate to bin/ and build the project
cd bin
make clean
make

# Start gdb with the compiled binary
gdb -q ./CmOS <<EOF
# Chose the line at which to break

break os.c:19

# Run the program
run


EOF
