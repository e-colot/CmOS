# printf "\n"
# echo "Usage: ./compile.sh <file>"
# printf "\n"

INPUT="src/"$1
OUTPUT="bin/$(basename "$INPUT")"

# Run the compiler
rm -f "$OUTPUT"

cd ./compiler
python3 compiler.py "$INPUT" "$OUTPUT"

echo "Compiled: $OUTPUT"
