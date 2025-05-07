#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "devTools.h"
#include "fileSystem.h"
#include "contiguousAllocation.h"
#include "disk.h"
#include "measurements.h"


unsigned char setFiles(FileEntry* fileEntries, size_t* entriesLength, FileInfo* files, size_t filesLength) {
    // adds a random number of files to the disk
    // return 0 if the disk is full (if addFile failed)
    size_t filesAdded = (rand() % 20) + 5;

    printf("Attempting to add %zu files to the disk...\n", filesAdded);

    for (size_t i = 0; i < filesAdded; i++) {
        // generate an ID
        AddressType ID;
        ID.value = rand() % 0xFFFFFFFF;
        // set to 0 the bytes that are not used for addressing
        for (size_t j = 3; j >= ADDRESSING_BYTES; j--) {
            ID.bytes[j] = 0;
        }
        // check that the ID is not 0
        if (ID.value == 0) {
            i--;
            continue;
        }
        // check that the ID is not already used
        unsigned char valid = 1;
        for (size_t j = 0; j < *entriesLength; j++) {
            if (ID.value == (fileEntries + j)->ID.value) {
                valid = 0;
                i--;
                break;
            }
        }
        if (!valid) {
            continue;
        }
        // add the file to the disk
        size_t fileIndex = rand() % filesLength;
        printf("    Adding file: %s with ID: ", files[fileIndex].fileName);
        printAddress(ID);
        printf("...");
        if (addFile(files[fileIndex].fileName, ID)) {
            printf("\033[33m        Disk is full\033[0m\n");
            return 1;
        } else {
            // add the file to the entries
            fileEntries[*entriesLength].fileIndex = fileIndex;
            fileEntries[*entriesLength].ID = ID;
            *entriesLength = *entriesLength + 1;
            printf("\033[32m        Success\033[0m\n");
        }
    }

    printf("Successfully added %zu files to the disk.\n\n", filesAdded);
    return 0;
}

unsigned char checkFile(FileEntry* fileEntries, size_t* entriesLength, FileInfo* files) {
    // check if the files are correctly added
    printf("Checking files...\n");
    for (size_t i = 0; i < *entriesLength; i++) {
        printf("    Checking file: %s with ID: ", files[fileEntries[i].fileIndex].fileName);
        printAddress(fileEntries[i].ID);
        printf("...");
        unsigned char* buffer = malloc(files[fileEntries[i].fileIndex].fileSize);
        if (loadFile(fileEntries[i].ID, buffer, files[fileEntries[i].fileIndex].fileSize)) {
            printf("\033[31m\nError loading file: %s\033[0m\n", files[fileEntries[i].fileIndex].fileName);
            free(buffer);
            return 1;
        }
        // check whether the file was correctly added
        FILE* file = fopen(files[fileEntries[i].fileIndex].fileName, "rb");
        if (!file) {
            printf("\033[31m\nFailed to open file: %s\033[0m\n", files[fileEntries[i].fileIndex].fileName);
            free(buffer);
            return 1;
        }
        // read the first, last, and 10 random bytes from the file and check if they are the same
        unsigned char error = 0;

        // Check the first byte
        unsigned char firstByte;
        fseek(file, 0, SEEK_SET);
        fread(&firstByte, sizeof(unsigned char), 1, file);
        if (buffer[0] != firstByte) {
            printf("\033[33m\nError reading first byte\033[0m\n");
            error = 1;
        }

        // Check the last byte
        unsigned char lastByte;
        fseek(file, files[fileEntries[i].fileIndex].fileSize - 1, SEEK_SET);
        fread(&lastByte, sizeof(unsigned char), 1, file);
        if (buffer[files[fileEntries[i].fileIndex].fileSize - 1] != lastByte) {
            printf("\033[33m\nError reading last byte\033[0m\n");
            error = 1;
        }

        // Check 10 random bytes
        for (size_t j = 0; j < 10; j++) {
            size_t randomIndex = rand() % files[fileEntries[i].fileIndex].fileSize;
            unsigned char randomByte;
            fseek(file, randomIndex, SEEK_SET);
            fread(&randomByte, sizeof(unsigned char), 1, file);
            if (buffer[randomIndex] != randomByte) {
                printf("\033[33m\nError reading byte %zu out of %zu bytes\033[0m\n", randomIndex, files[fileEntries[i].fileIndex].fileSize);
                error = 1;
                break;
            }
        }
        fclose(file);
        free(buffer);
        if (error) {
            printf("\033[31m\nFile not correctly written/read: %s\033[0m\n", files[fileEntries[i].fileIndex].fileName);
            return 1;
        }
        printf("\033[32m        Success\033[0m\n");
    }
    printf("All files successfully verified.\n\n");
    return 0;
}

unsigned char delFiles(FileEntry* fileEntries, size_t* entriesLength) {
    // delete a random number of files
    size_t filesToDelete = rand() % *entriesLength;
    printf("Attempting to delete %zu files...\n", filesToDelete);

    for (size_t i = 0; i < filesToDelete; i++) {
        // generate a random index
        size_t randomIndex = rand() % *entriesLength;
        printf("    Attempting to delete file with ID: ");
        printAddress(fileEntries[randomIndex].ID);
        printf("...");

        if (removeFile(fileEntries[randomIndex].ID)) {
            printf("\033[31m\nError removing file\033[0m\n");
            return 1;
        }
        printf("\033[32m        Success\033[0m\n");

        // remove the file from the entries
        for (size_t j = randomIndex; j < *entriesLength - 1; j++) {
            fileEntries[j] = fileEntries[j + 1];
        }
        *entriesLength = *entriesLength - 1;
    }

    size_t FATsize = getFATsize();
    if (FATsize != *entriesLength) {
        printf("\033[31mError: FAT size is not equal to the number of files\033[0m\n");
        return 1;
    }

    printf("Successfully deleted %zu files.\n\n", filesToDelete);
    return 0;
}

void runTest() {

    // file creation
    size_t fileNbr = 5;
    // means that there will never be more than 100 files
    size_t maxFileCnt = 100;
    size_t minFileSize = DISK_SIZE/(maxFileCnt);
    size_t fileSizes[fileNbr];
    for (size_t i = 0; i < 5; i++) {
        fileSizes[i] = minFileSize + i + rand() % (PAGE_SIZE/2);
    }
    FileInfo fileInfos[fileNbr];
    createFiles(fileSizes, fileNbr, fileInfos);

    size_t* length = malloc(sizeof(size_t));
    *length = 0;
    FileEntry* fileEntries = malloc(maxFileCnt*3/2 * sizeof(FileEntry));

    // add files to the disk 
    // No error catch as a disk full error can occur and has no impact on the test
    setFiles(fileEntries, length, fileInfos, fileNbr);

    // remove files from the disk
    if (delFiles(fileEntries, length)) {
        printf("Error removing files\n");
        free(fileEntries);
        free(length);
        return;
    }

    // check if the files are correctly added
    if (checkFile(fileEntries, length, fileInfos)) {
        printf("Error checking files\n");
        free(fileEntries);
        free(length);
        return;
    }

    // fill the disk
    while (1) {
        if (setFiles(fileEntries, length, fileInfos, fileNbr)) {
            // disk is full
            break;
        }
    }

    //check again
    if (checkFile(fileEntries, length, fileInfos)) {
        printf("Error checking files\n");
        free(fileEntries);
        free(length);
        return;
    }

    // remove files from the disk
    if (delFiles(fileEntries, length)) {
        printf("Error removing files\n");
        free(fileEntries);
        free(length);
        return;
    }

    // add files to the disk one more time
    // No error catch as a disk full error can occur and has no impact on the test
    setFiles(fileEntries, length, fileInfos, fileNbr);

    // check if the files are correctly added
    if (checkFile(fileEntries, length, fileInfos)) {
        printf("Error checking files\n");
        free(fileEntries);
        free(length);
        return;
    }

    // if no error, print success message
    printf("All tests passed\n");

    // free memory
    for (size_t i = 0; i < fileNbr; i++) {
        free(fileInfos[i].fileName);
    }
    free(fileEntries);
    free(length);
}
