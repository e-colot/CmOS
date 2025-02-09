#include "devTools.h"
#include <stdio.h>

void printCharList(char* list, size_t len) {
    // Iterate over each element in the list and print its value
    for (size_t i = 0; i < len; i++) {
        printf("%d ", (unsigned char)list[i]); // Print the integer (ASCII value) of each character
    }
    printf("\n"); // Newline at the end for formatting
}

void printReg(Reg* reg) {
    printf("FLAGS: %d\n", *(reg->FLAGS));
    printf("R1: %d\n", *(reg->R1));
    printf("R2: %d\n", *(reg->R2));
    printf("R3: %d\n", *(reg->R3));
    printf("R4: %d\n", *(reg->R4));
    printf("R5: %d\n", *(reg->R5));
    printf("RL: %d\n", *(reg->RL));
    printf("RH: %d\n", *(reg->RH));
    printf("R16: %d\n", *(reg->R16));
    printf("RSI: %d\n", *(reg->RSI));
    printf("RDI: %d\n", *(reg->RDI));
    printf("RI: %d\n", *(reg->RI));
    printf("RS: %d\n\n", *(reg->RS));
}
