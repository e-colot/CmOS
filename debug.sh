#!/bin/bash

# Clear terminal
clear

# Navigate to bin/ and build the project
cd bin
make clean
make

clear

# Check for arguments and decide how to debug
if [[ "$1" == "--test" ]]; then
    gdb -q -args ./CmOS --test
else
    gdb -q ./CmOS
fi

