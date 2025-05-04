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

# Run the OS binary with --test argument
./CmOS --test

printf "\n\n";
