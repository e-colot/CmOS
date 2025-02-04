#include "devTools.h"
#include <stdio.h>

void printCharList(char* list, size_t len) {
    // Iterate over each element in the list and print its value
    for (size_t i = 0; i < len; i++) {
        printf("%d ", (unsigned char)list[i]); // Print the integer (ASCII value) of each character
    }
    printf("\n"); // Newline at the end for formatting
}
