#include "devTools.h"
#include "constants.h"
#include <stdio.h>

#include "disk.h" // For diskRead

void printCharList(unsigned char* list, size_t len) {
    // Iterate over each element in the list and print its value
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", (unsigned char)list[i]); // Print the integer (hex value) of each character
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

void printMem(Ram* memory) {
    printCharList(memory->mem, RAM_SIZE);
}

void printBitmap() {
    unsigned char* bitmap = malloc(DISK_SIZE/(8*PAGE_SIZE));
    diskRead(0, bitmap, 0, DISK_SIZE/(8*PAGE_SIZE));
    for (size_t i = 0; i < DISK_SIZE/(8*PAGE_SIZE); i++) {
        // in binary
        printf("%d%d%d%d%d%d%d%d  ", 
            (bitmap[i] & 0b10000000) >> 7, 
            (bitmap[i] & 0b01000000) >> 6, 
            (bitmap[i] & 0b00100000) >> 5, 
            (bitmap[i] & 0b00010000) >> 4, 
            (bitmap[i] & 0b00001000) >> 3, 
            (bitmap[i] & 0b00000100) >> 2, 
            (bitmap[i] & 0b00000010) >> 1, 
            (bitmap[i] & 0b00000001));
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    free(bitmap);
}

void printFAT() {
    unsigned char* fat = malloc(PAGE_SIZE);
    // page index starts after the bitmap pages
    // the bitmap takes DISK_SIZE/(8*PAGE_SIZE)
    // this corresponds to DISK_SIZE/(8*PAGE_SIZE*PAGE_SIZE) pages
    unsigned char pageIndex = DISK_SIZE/(8*PAGE_SIZE*PAGE_SIZE);
    while (pageIndex) {
        diskRead(pageIndex*PAGE_SIZE, fat, 0, PAGE_SIZE);
        for (size_t i = 0; i < PAGE_SIZE; i++) {
            printf("%02X ", fat[i]);
            if (i % 2 == 1) {
                printf("\n");
            }
        }
        printf("\nEnd of FAT page\n\n");
        pageIndex = fat[1];
    }
    free(fat);
}
