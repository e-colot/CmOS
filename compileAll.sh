#!/bin/bash

# Change directory to programs/
cd programs/bin
rm -f *.machineCode
cd ..

printf "\n"

# Loop through all files in src/ and run ./compile.sh for each
for file in ./src/*; 
do
    # Extract the filename without the path
    filename=$(basename "$file")
    
    # Run ./compile.sh with the filename as argument
    ./compile.sh "$filename"
done

cd ..

printf "\n"
