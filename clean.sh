#!/bin/bash

# Navigate to the bin directory
cd bin

# Call the 'clean' rule in the Makefile
make clean
clear

# Print a message indicating the clean process is complete
printf "Clean complete. All build artifacts have been removed."

printf "\n\n\n";
