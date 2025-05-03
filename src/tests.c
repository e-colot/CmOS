#include "tests.h"

#include "fileSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "devTools.h"

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
    if (loadFile(IDs[rand() % fileNbr], buffer, adjustedFileSize)) {
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




