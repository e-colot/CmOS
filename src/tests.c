#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "devTools.h"
#include "fileSystem.h"
#include "disk.h"

unsigned char writeTest(size_t fileNbr, size_t fileSize) {
    
    // randomness initialization
    static int seedInitialized = 0;
    if (!seedInitialized) {
        srand(time(NULL));
        seedInitialized = 1;
    }

    // file creation
    unsigned char* fileContent = malloc(fileSize);
    FILE *file = fopen("../programs/bin/randomFile", "wb");
    if (!file) {
        printf("Failed to create file\n");
        return 1;
    }
    for (size_t i = 0; i < fileSize; i++) {
        unsigned char randomByte = rand() % 0xFF;
        fileContent[i] = randomByte;
        fwrite(&randomByte, sizeof(unsigned char), 1, file);
    }
    fclose(file);

    // creation of the ID's
    AddressType* IDs = malloc(fileNbr * sizeof(AddressType));
    for (size_t i = 0; i < fileNbr; i++) {
        IDs[i].value = rand() % 0xFFFFFFFF;
        // set to 0 the bytes that are not used for addressing
        for (size_t j = 3; j >= ADDRESSING_BYTES; j--) {
            IDs[i].bytes[j] = 0;
        }
        // check that the ID is not already used
        for (size_t j = 0; j < i; j++) {
            if (IDs[i].value == IDs[j].value) {
                i--;
                break;
            }
        }
    }
    // file addition
    for (size_t i = 0; i < fileNbr; i++) {
        if(addFile("../programs/bin/randomFile", IDs[i])) {
            printf("Error adding file\n");
            free(fileContent);
            return 1;
        }
    }

    unsigned char error = 0;
    // adjustedFileSize as fileSize is not a multiple of PAGE_SIZE
    size_t adjustedFileSize = fileSize/(PAGE_SIZE - ADDRESSING_BYTES);
    if (fileSize % (PAGE_SIZE-ADDRESSING_BYTES) != 0) {
        adjustedFileSize ++;
    }
    adjustedFileSize *= (PAGE_SIZE);
    // load a random copy of the file
    unsigned char* buffer = malloc(adjustedFileSize);
    AddressType randomID;
    randomID = IDs[rand() % fileNbr];
    if (loadFile(randomID, buffer, adjustedFileSize)) {
        printf("Error loading file\n");
        free(buffer);
        free(fileContent);
        return 1;
    }
    // check whether the file was correctly added
    for (size_t i = 0; i < fileSize; i++) {
        if (buffer[i] != fileContent[i]) {
            error = 1;
            break;
        }
    }
    free(buffer);
    free(fileContent);
    if (error) {
        printf("File not correctly written/read\n");
        return 1;
    }
    return 0;
}

unsigned char eraseTest(size_t fileNbr, size_t fileSize) {
    // randomness initialization
    static int seedInitialized = 0;
    if (!seedInitialized) {
        srand(time(NULL));
        seedInitialized = 1;
    }

    // file creation
    unsigned char* fileContent = malloc(fileSize);
    FILE *file = fopen("../programs/bin/randomFile", "wb");
    if (!file) {
        printf("Failed to create file\n");
        return 1;
    }
    for (size_t i = 0; i < fileSize; i++) {
        unsigned char randomByte = rand() % 0xFF;
        fileContent[i] = randomByte;
        fwrite(&randomByte, sizeof(unsigned char), 1, file);
    }
    free(fileContent);
    fclose(file);

    // creation of the ID's of 2*fileNbr files
    AddressType* IDs = malloc(2*fileNbr * sizeof(AddressType));
    for (size_t i = 0; i < 2*fileNbr; i++) {
        IDs[i].value = rand() % 0xFFFFFFFF;
        // set to 0 the bytes that are not used for addressing
        for (size_t j = 3; j >= ADDRESSING_BYTES; j--) {
            IDs[i].bytes[j] = 0;
        }
        // check that the ID is not already used
        for (size_t j = 0; j < i; j++) {
            if (IDs[i].value == IDs[j].value) {
                i--;
                break;
            }
        }
    }

    // file addition
    for (size_t i = 0; i < 2*fileNbr; i++) {
        if(addFile("../programs/bin/randomFile", IDs[i])) {
            printf("Error adding file\n");
            return 1;
        }
    }

    // erase fileNbr files
    for (size_t i = 0; i < fileNbr; i++) {
        size_t randomIndex = rand() % (2 * fileNbr);
        while (IDs[randomIndex].value == 0) {
            randomIndex = rand() % (2 * fileNbr);
        }

        if (removeFile(IDs[randomIndex])) {
            printf("Error removing file\n");
            return 1;
        }

        //check in FAT if the file was removed
        unsigned char* fat = malloc(PAGE_SIZE);
        AddressType pageIndex;
        pageIndex.value = BITMAP_PAGES;
        while (pageIndex.value) {
            // interpret it as "while the next FAT page is not in 0"
            diskRead(pageIndex.value*PAGE_SIZE, fat, 0, PAGE_SIZE);
            AddressType ID;
            ID.value = 0;
            for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
                for (unsigned char j = 0; j < ADDRESSING_BYTES; j++) {
                    ID.bytes[j] = *(fat + i*ADDRESSING_BYTES + j);
                }
            }
            // check if the ID is the one we are looking for
            if (ID.value == IDs[randomIndex].value) {
                printf("File not removed from FAT\n");
                free(fat);
                return 1;
            }
            // load next FAT page address
            pageIndex = getAddress(fat + ADDRESSING_BYTES);
        }
        free(fat);

        IDs[randomIndex].value = 0; // Mark as used
    }

    free(IDs);
    return 0;
}
    



