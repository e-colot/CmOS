#include "measurements.h"
#include "constants.h"
#include "fileSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>



void createFiles(size_t* sizes, size_t count, FileInfo* fileInfos) {
    for (size_t i = 0; i < count; i++) {
        // Generate file name
        char filePath[50];
        snprintf(filePath, sizeof(filePath), "./file%zu", i + 1);

        // Open file for writing
        FILE* file = fopen(filePath, "wb");
        if (!file) {
            printf("\033[31mFailed to create file %s\033[0m\n", filePath);
            continue;
        }

        // Write random content
        size_t fileSize = sizes[i];
        for (size_t j = 0; j < fileSize; j++) {
            unsigned char randomByte = rand() % 0xFF;
            fwrite(&randomByte, sizeof(unsigned char), 1, file);
        }
        fclose(file);

        // Store file information
        fileInfos[i].fileName = strdup(filePath);
        fileInfos[i].fileSize = fileSize;
    }
}

unsigned char fillDisk(FileEntry* fileEntries, size_t* entriesLength, FileInfo* files, size_t filesLength) {
    // adds files to the disk until the disk is full
    // return 0 if the disk is full (if addFile failed)

    for (size_t i = 0;; i++) {
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
        if (addFile(files[fileIndex].fileName, ID)) {
            // disk is full
            return 1;
        } else {
            // add the file to the entries
            fileEntries[*entriesLength].fileIndex = fileIndex;
            fileEntries[*entriesLength].ID = ID;
            *entriesLength = *entriesLength + 1;
        }
    }
}

unsigned char emptyDisk(FileEntry* fileEntries, size_t* entriesLength) {
    // delete a random number of files
    size_t filesToDelete = *entriesLength;

    for (size_t i = 0; i < filesToDelete; i++) {
        // generate a random index
        size_t randomIndex = rand() % *entriesLength;

        if (removeFile(fileEntries[randomIndex].ID)) {
            printf("\033[31m\nError removing file\033[0m\n");
            return 1;
        }

        // remove the file from the entries
        for (size_t j = randomIndex; j < *entriesLength - 1; j++) {
            fileEntries[j] = fileEntries[j + 1];
        }
        *entriesLength = *entriesLength - 1;
    }
    return 0;
}


size_t writeDataToFile(const char* fileName, const unsigned char* data, size_t dataSize) {
    FILE* file = fopen(fileName, "wb");
    if (!file) {
        printf("Error opening file for writing: %s\n", fileName);
        return 0;
    }

    size_t bytesWritten = fwrite(data, sizeof(unsigned char), dataSize, file);

    fclose(file);

    if (bytesWritten != dataSize) {
        printf("Error writing data to file: %s\n", fileName);
        return 0;
    }

    return bytesWritten;
}

unsigned char timeToFillDisk(const char* fileName, size_t itr, unsigned char createNewFiles) {
    printf("Starting timeToFillDisk function\n");

    // create the data buffer
    unsigned char* fileBuffer = malloc(4*sizeof(size_t) + 2*itr*sizeof(double));
    if (fileBuffer == NULL) {
        printf("\033[31mError: Failed to allocate memory for buffer\033[0m\n");
        return 1;
    }
    size_t index = 0;

    size_t diskSize = DISK_SIZE;
    size_t pageSize = PAGE_SIZE;
    size_t fileAlloc = FILE_ALLOCATION;

    // write the disk size, page size and file allocation to the file
    printf("Adding system parameters...\n");
    memcpy(fileBuffer + index, &diskSize, sizeof(size_t));
    index += sizeof(size_t);
    memcpy(fileBuffer + index, &pageSize, sizeof(size_t));
    index += sizeof(size_t);
    memcpy(fileBuffer + index, &fileAlloc, sizeof(size_t));
    index += sizeof(size_t);
    printf("\033[32m        Done\033[0m\n");

    // file creation
    printf("File creation:\n");
    size_t fileNbr = 10;
    size_t minFileSize = 500;
    size_t maxFileSize = 10*minFileSize;
    FileInfo fileInfos[fileNbr];
    size_t avgFileSize = 0;
    if (createNewFiles) {
        printf("    Trying to create new files...\n");
        size_t fileSizes[fileNbr];

        for (size_t i = 0; i < fileNbr; i++) {
            fileSizes[i] = minFileSize + rand() % (maxFileSize - minFileSize);
            avgFileSize += fileSizes[i];
        }
        avgFileSize /= fileNbr;

        createFiles(fileSizes, fileNbr, fileInfos);
        printf("\033[32m        Success\033[0m\n");
    }
    else {
        printf("    Recovering existing files...\n");
        for (size_t i = 0; i < fileNbr; i++) {
            // generate file name
            char filePath[50];
            snprintf(filePath, sizeof(filePath), "./file%zu", i + 1);
            // store file information
            fileInfos[i].fileName = strdup(filePath);
            struct stat st;
            if (stat(filePath, &st) == 0) {
                fileInfos[i].fileSize = st.st_size;
                avgFileSize += fileInfos[i].fileSize;
                printf("        File %zu: %s, size: %zu bytes\n", i + 1, fileInfos[i].fileName, fileInfos[i].fileSize);
            } 
            else {
                printf("\033[31m        Error: Unable to get file size for %s\033[0m\n", filePath);
                printf("        Error: Need to create new files\n");
                free(fileBuffer);
                return 1;
            }
        }
        avgFileSize /= fileNbr;
        printf("        Average file size: %zu bytes\n", avgFileSize);
        printf("\033[32m        Success\033[0m\n");
    }
    // write the avg file size to the file
    memcpy(fileBuffer + index, &avgFileSize, sizeof(size_t));
    index += sizeof(size_t);

    size_t* length = malloc(sizeof(size_t));
    *length = 0;
    FileEntry* fileEntries = malloc(DISK_SIZE/minFileSize * sizeof(FileEntry));

    for (size_t i = 0; i < itr; i++) {
        printf("Iteration %zu/%zu: Trying to fill and empty disk...\n", i + 1, itr);

        // clock the time taken to fill the disk
        printf("    Filling disk... ");

        clock_t startFill = clock();
        fillDisk(fileEntries, length, fileInfos, fileNbr);
        clock_t endFill = clock();
        double timeTakenFill = (double)(endFill - startFill) / CLOCKS_PER_SEC;

        printf("\033[32m        Success\033[0m\n");
        printf("    Emptying disk...");

        clock_t startDel = clock();
        emptyDisk(fileEntries, length);
        clock_t endDel = clock();
        double timeTakenDel = (double)(endDel - startDel) / CLOCKS_PER_SEC;

        printf("\033[32m        Success\033[0m\n");

        // Write the times to the fileBuffer
        memcpy(fileBuffer + index, &timeTakenFill, sizeof(double));
        index += sizeof(double);
        memcpy(fileBuffer + index, &timeTakenDel, sizeof(double));
        index += sizeof(double);
    }

    // write the file buffer to the file
    printf("Writing buffer to file: %s...\n", fileName);
    size_t bytesWritten = writeDataToFile(fileName, fileBuffer, index);
    if (bytesWritten != index) {
        printf("\033[31m        Error: Failed to write data to file\033[0m\n");
        free(fileBuffer);
        return 1;
    }
    printf("\033[32m        Success\033[0m\n");

    // clean exit
    printf("Cleaning up and exiting...\n");
    free(fileBuffer);
    free(fileEntries);
    free(length);
    printf("\033[32m        Success\033[0m\n");
    return 0;
}

