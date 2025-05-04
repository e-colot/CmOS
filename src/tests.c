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
        free(fileContent);
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
            free(IDs);
            return 1;
        }
    }

    unsigned char error = 0;
    // adjustedFileSize as fileSize is not a multiple of PAGE_SIZE
    size_t adjustedFileSize;
    if (FILE_ALLOCATION == 0) {
        adjustedFileSize = (fileSize+PAGE_SIZE-ADDRESSING_BYTES-1)/(PAGE_SIZE-ADDRESSING_BYTES);
    }
    else {
        adjustedFileSize = (fileSize+PAGE_SIZE-1)/PAGE_SIZE;
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
        free(IDs);
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
    free(IDs);
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
    FILE *file = fopen("../programs/bin/randomFile", "wb");
    if (!file) {
        printf("Failed to create file\n");
        return 1;
    }
    for (size_t i = 0; i < fileSize; i++) {
        unsigned char randomByte = rand() % 0xFF;
        fwrite(&randomByte, sizeof(unsigned char), 1, file);
    }
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
            free(IDs);
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
            free(IDs);
            return 1;
        }

        //check in FAT if the file was removed
        if(searchFAT(IDs[randomIndex]).value != 0) {
            printf("File not correctly removed");
            free(IDs);
            return 1;
        }

        IDs[randomIndex].value = 0; // Mark as used
    }

    free(IDs);
    return 0;
}
    
unsigned char fatFillingTest(size_t fileNbr, size_t fileSize) {
    // randomness initialization
    static int seedInitialized = 0;
    if (!seedInitialized) {
        srand(time(NULL));
        seedInitialized = 1;
    }

    // file creation
    FILE *file = fopen("../programs/bin/randomFile", "wb");
    if (!file) {
        printf("Failed to create file\n");
        return 1;
    }
    for (size_t i = 0; i < fileSize; i++) {
        unsigned char randomByte = rand() % 0xFF;
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
            free(IDs);
            return 1;
        }
    }

    // check if each element of IDs is in the FAT
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not in 0"
        diskRead(pageIndex.value*PAGE_SIZE, fat, 0, PAGE_SIZE);
        AddressType ID;
        ID.value = 0;
        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
            for (unsigned char j = 0; j < ADDRESSING_BYTES; j++) {
                ID.bytes[j] = *(fat + i*ADDRESSING_BYTES + j);
            }
            // check if the ID is in the IDs array
            for (size_t j = 0; j < fileNbr; j++) {
                if (ID.value == IDs[j].value) {
                    IDs[j].value = 0; // Mark as used
                    break;
                }
            }
        }
        // load next FAT page address
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
    }
    // check if all IDs were found
    for (size_t i = 0; i < fileNbr; i++) {
        if (IDs[i].value != 0) {
            printf("File not found in FAT\n");
            free(fat);
            free(IDs);
            return 1;
        }
    }
    free(fat);
    free(IDs);
    return 0;
}

unsigned char fatReorganizationTest(size_t fileNbr, size_t fileSize) {
    // simply call eraseTest as it mess up the FAT
    if(eraseTest(fileNbr, fileSize)) {
        printf("Error in eraseTest\n");
        return 1;
    }

    reorganizeFAT();

    // check every FAT page (except the last one) to see if any entry is empty
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not in 0"
        diskRead(pageIndex.value*PAGE_SIZE, fat, 0, PAGE_SIZE);
        // load next FAT page address
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
        if (pageIndex.value == 0) {
            // if it is the last FAT page, do not check it as it can contain empty entries
            break;
        }

        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
            if (checkAddress(fat + i*ADDRESSING_BYTES, 0)) {
                printf("Empty entry in FAT\n");
                free(fat);
                return 1;
            }
        }
    }

    free(fat);
    return 0;
}

void runTests() {
    printf("Running tests with:\n");
    printf("File number: %zu\n", (size_t)TEST_FILE_NBR);
    if (TEST_FILE_SIZE < 1024) {
        printf("File size: %zu B\n\n", (size_t)TEST_FILE_SIZE);
    } else if (TEST_FILE_SIZE < 1024 * 1024) {
        printf("File size: %zu kB\n\n", (size_t)(TEST_FILE_SIZE / 1024));
    } else {
        printf("File size: %zu MB\n\n", (size_t)(TEST_FILE_SIZE / (1024 * 1024)));
    }
    if (writeTest(TEST_FILE_NBR, TEST_FILE_SIZE)) {
        printf("Error in Write test\n");
        return;
    }
    else {
        printf("Write test succeeded\n");
    }
    diskInit(DISK_SIZE); // to avoid overfilling and duplicate ID's
    if (eraseTest(TEST_FILE_NBR, TEST_FILE_SIZE)) {
        printf("Error in Erase test\n");
        return;
    }
    else {
        printf("Erase test succeeded\n");
    }
    diskInit(DISK_SIZE);
    if (fatFillingTest(TEST_FILE_NBR, TEST_FILE_SIZE)) {
        printf("Error in FAT filling test\n");
        return;
    }
    else {
        printf("FAT filling test succeeded\n");
    }
    diskInit(DISK_SIZE);
    if (fatReorganizationTest(TEST_FILE_NBR, TEST_FILE_SIZE)) {
        printf("Error in FAT reorganization test\n");
        return;
    }
    else {
        printf("FAT reorganization test succeeded\n");
    }
    printf("\nAll tests passed\n\n");
}

