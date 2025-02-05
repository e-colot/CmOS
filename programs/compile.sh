echo "Usage: ./compile.sh <file>"

INPUT="src/"$1
OUTPUT="bin/$(basename "$INPUT").machineCode"

# Build the compiler
cd ./compiler
make
cd ..

# Run the compiler
./compiler/compiler "$INPUT" "$OUTPUT"

echo "Compiled: $OUTPUT"
