#!/bin/bash

# Clear terminal
clear

# Navigate to bin/ and run make
cd bin

make clean
make

printf "\n\n";

# Wait for user input
read -p "Press any key to continue..." -n1 -s

# Clear screen again
clear

# Check for arguments and decide how to run
if [[ "$1" == "--test" ]]; then
    ./CmOS --test
else
    ./CmOS
fi

printf "\n\n";
