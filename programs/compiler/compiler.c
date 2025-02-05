//TO CONTINUE BUT LOW PRIORITY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>    // For open()
#include <unistd.h>   // For write() and close()

#define MAX_LINE_LENGTH 24

char convertLine(char* instruction);
char matchingStr(char* str1, char* str2);

void compile(const char *input_file, const char *output_file) {

    FILE *in = fopen(input_file, "r");
    if (!in) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    int out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        perror("Error creating output file");
        fclose(in);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), in)) {

        // Remove newline characters if present
        line[strcspn(line, "\r\n")] = 0;

        // Print the line being processed (debugging)
        printf("Processing: %s\nOf length: %ld\n", line, strlen(line));

        convertLine(line);
        // TODO: Tokenize and convert instruction to machine code
        // Example: If the line is "MOV R1, 5", we should convert it into binary

        // Dummy machine code output (for now, just writing "0x00" for each line)
        const char *dummy_machine_code = "0x00\n";
        write(out, dummy_machine_code, strlen(dummy_machine_code));
    }

}

char convertLine(char* instruction) {
    char *opt[] = {"AND", "OR", "NOT", "SHL", "SHR", "ADD", "SUB", "MUL", "IDIV", "MOD", "MOV", "LOAD", "STORE", "REGDUMP", "REGFILL", "CMP", "TEST", "SKIZ", "SKINZ", "PRNT", "HLT"};
    char lengthOpt = sizeof(opt)/sizeof(opt[0]);
    char success = 0;
    for(unsigned char i = 0; i < lengthOpt; i++) {
        if(matchingStr(instruction, opt[i])) {
            // do some shit
            success = 1;
            break;
        }
    }

    if(success == 0){
        printf("Unknown instruction for: %s\n", instruction);
    }
    
    return 0;
}

char matchingStr(char* str1, char* str2) {
    // returns 0 if not matching
    char minLen = (strlen(str1) > strlen(str2)) ? strlen(str2) : strlen(str1);
    unsigned char pos = 0;
    char out = 1;
    while(pos < minLen) {
        if (str1[pos] != str2[pos]) {
            out = 0;
            break;
        }
        pos++;
    }
    return (out*minLen);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.myasm> <output.machineCode>\n", argv[0]);
        return EXIT_FAILURE;
    }

    compile(argv[1], argv[2]);
    return EXIT_SUCCESS;
}
