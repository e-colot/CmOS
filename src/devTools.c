#include "devTools.h"
#include "constants.h"
#include "fileSystem.h"
#include "contiguousAllocation.h"
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
    unsigned char* bitmap = malloc(BITMAP_SIZE);
    diskRead(0, bitmap, 0, BITMAP_SIZE);
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
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
    if (FILE_ALLOCATION == 0) {
        unsigned char* fat = malloc(PAGE_SIZE);
        // page index starts after the bitmap pages
        // the bitmap takes DISK_SIZE/(8*PAGE_SIZE)
        // this corresponds to DISK_SIZE/(8*PAGE_SIZE*PAGE_SIZE) pages
        AddressType pageIndex;
        pageIndex.value = BITMAP_PAGES;
        while (pageIndex.value) {
            // interpret it as "while the next FAT page is not in 0"
            diskRead(pageIndex.value*PAGE_SIZE, fat, 0, PAGE_SIZE);
            for (size_t i = 0; i < PAGE_SIZE; i++) {
                printf("%02X", fat[i]);
                if (i % ADDRESSING_BYTES == ADDRESSING_BYTES - 1) {
                    printf(" ");
                }
                if (i % (2*ADDRESSING_BYTES) == 2*ADDRESSING_BYTES - 1) {
                    printf("\n");
                }
            }
            if (PAGE_SIZE % (2*ADDRESSING_BYTES) != 0) {
                printf("\n");
            }
            printf("End of FAT page located at 0x%x\n", pageIndex.value);
            pageIndex = getAddress(fat + ADDRESSING_BYTES);
        }
        free(fat);
        printf("End of FAT\n\n");
    } 
    else {
        AddressType index = {0};
        CA_FATEntry entry = CA_getFATEntry(index);
        for (size_t i = entry.length.value; i+1 > 0; i--) {
            // for each entry in the FAT

            // show the entry ID
            printf("ID: ");
            for (size_t j = 0; j < ADDRESSING_BYTES; j++) {
                printf("%02X", entry.ID.bytes[j]);
            }
            printf("    ");
            // show the entry page
            printf("Page: ");
            for (size_t j = 0; j < ADDRESSING_BYTES; j++) {
                printf("%02X", entry.page.bytes[j]);
            }
            printf("    ");
            // show the entry length
            printf("Length: ");
            for (size_t j = 0; j < ADDRESSING_BYTES; j++) {
                printf("%02X", entry.length.bytes[j]);
            }
            printf("\n");
            // get the next entry
            index.value = index.value + 1;
            entry = CA_getFATEntry(index);
        }

    }
}
