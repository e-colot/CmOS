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

typedef struct {
    char* fileName;
    size_t fileSize;
} FileInfo;

typedef struct {
    unsigned char fileIndex;
    AddressType ID;
} FileEntry;

void createFiles(size_t* sizes, size_t count, FileInfo* fileInfos) {
    for (size_t i = 0; i < count; i++) {
        // Generate file name
        char filePath[50];
        snprintf(filePath, sizeof(filePath), "../programs/bin/file%zu", i + 1);

        // Open file for writing
        FILE* file = fopen(filePath, "wb");
        if (!file) {
            printf("Failed to create file %s\n", filePath);
            continue;
        }

        // Write random content of size * PAGE_SIZE
        size_t fileSize = sizes[i] * PAGE_SIZE;
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
            printf("Disk is full\n");
            return 1;
        } else {
            // add the file to the entries
            fileEntries[*entriesLength].fileIndex = fileIndex;
            fileEntries[*entriesLength].ID = ID;
            *entriesLength = *entriesLength + 1;
            printf("    Successfully added file: %s with ID: %x\n", files[fileIndex].fileName, ID.value);
        }
    }

    printf("Successfully added %zu files to the disk.\n\n", filesAdded);
    return 0;
}

unsigned char checkFile(FileEntry* fileEntries, size_t* entriesLength, FileInfo* files) {
    // check if the files are correctly added
    printf("Checking files...\n");
    for (size_t i = 0; i < *entriesLength; i++) {
        printf("    Checking file: %s with ID: %x\n", files[fileEntries[i].fileIndex].fileName, fileEntries[i].ID.value);
        unsigned char* buffer = malloc(files[fileEntries[i].fileIndex].fileSize);
        if (loadFile(fileEntries[i].ID, buffer, files[fileEntries[i].fileIndex].fileSize)) {
            printf("    Error loading file: %s\n", files[fileEntries[i].fileIndex].fileName);
            free(buffer);
            return 1;
        }
        // check whether the file was correctly added
        FILE* file = fopen(files[fileEntries[i].fileIndex].fileName, "rb");
        if (!file) {
            printf("    Failed to open file: %s\n", files[fileEntries[i].fileIndex].fileName);
            free(buffer);
            return 1;
        }
        // read 10 random bytes from the file and check if they are the same
        unsigned char error = 0;
        for (size_t j = 0; j < 10; j++) {
            size_t randomIndex = rand() % files[fileEntries[i].fileIndex].fileSize;
            unsigned char randomByte;
            fseek(file, randomIndex, SEEK_SET);
            fread(&randomByte, sizeof(unsigned char), 1, file);
            if (buffer[randomIndex] != randomByte) {
                error = 1;
                break;
            }
        }
        fclose(file);
        free(buffer);
        if (error) {
            printf("    File not correctly written/read: %s\n", files[fileEntries[i].fileIndex].fileName);
            return 1;
        }
        printf("    File successfully verified: %s\n", files[fileEntries[i].fileIndex].fileName);
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
        printf("    Attempting to delete file with ID: %x\n", fileEntries[randomIndex].ID.value);

        if (removeFile(fileEntries[randomIndex].ID)) {
            printf("    Error removing file with ID: %x\n", fileEntries[randomIndex].ID.value);
            return 1;
        }

        printf("    Successfully removed file with ID: %x\n", fileEntries[randomIndex].ID.value);

        // remove the file from the entries
        for (size_t j = randomIndex; j < *entriesLength - 1; j++) {
            fileEntries[j] = fileEntries[j + 1];
        }
        *entriesLength = *entriesLength - 1;
    }

    printf("Successfully deleted %zu files.\n\n", filesToDelete);
    return 0;
}

void runTest() {

    // file creation
    size_t fileNbr = 5;
    // means that there will never be more than 100 files
    size_t maxFileCnt = 100;
    size_t minFileSize = DISK_SIZE/(maxFileCnt*PAGE_SIZE);
    size_t fileSizes[fileNbr];
    for (size_t i = 0; i < 5; i++) {
        fileSizes[i] = minFileSize + i;
    }
    FileInfo fileInfos[fileNbr];
    createFiles(fileSizes, fileNbr, fileInfos);

    size_t* length = malloc(sizeof(size_t));
    *length = 0;
    FileEntry* fileEntries = malloc(maxFileCnt*3/2 * sizeof(FileEntry));

    // add files to the disk 
    if (setFiles(fileEntries, length, fileInfos, fileNbr)) {
        printf("Error adding files\n");
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
    if (setFiles(fileEntries, length, fileInfos, fileNbr)) {
        printf("Error adding files\n");
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

    // if no error, print success message
    printf("All tests passed\n");

}

//-------------------------------------------------------------------------------------------------

unsigned char writeTest(size_t fileNbr, size_t fileSize) {
    
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

unsigned char multipleWriteEraseTest(size_t maxMinItr) {
    // Create multiple files of varying sizes
    // Create 5 files with sizes as multiples of (PAGE_SIZE - ADDRESSING_BYTES)
    char filePaths[5][30];
    for (int i = 0; i < 5; i++) {
        snprintf(filePaths[i], sizeof(filePaths[i]), "../programs/bin/randomFile%d", i + 1);
        FILE *file = fopen(filePaths[i], "wb");
        if (!file) {
            printf("Failed to create file %s\n", filePaths[i]);
            return 1;
        }
        size_t fileSize = (DISK_SIZE*(4+i))/100;
        for (size_t j = 0; j < fileSize; j++) {
            unsigned char randomByte = rand() % 0xFF;
            fwrite(&randomByte, sizeof(unsigned char), 1, file);
        }
        fclose(file);
    }
    AddressType ID;
    ID.value = 1;
    AddressType* usedIDs = malloc((DISK_SIZE / PAGE_SIZE) * sizeof(AddressType));
    if (!usedIDs) {
        printf("Failed to allocate memory for usedIDs\n");
        return 1;
    }
    size_t usedIDCount = 0;

    size_t cnt = 0;
    for (; maxMinItr > 0; maxMinItr--) {
        printf("    Pass number %zu\n", ++cnt);
        // Add random files until the disk gets full
        while (1) {
            int randomIndex = rand() % 5;
            if (addFile(filePaths[randomIndex], ID)) {
                // disk is full
                break;
            }

            usedIDs[usedIDCount++] = ID; // Store the used ID
            // Generate a new unique ID
            do {
                ID.value = rand() % 0xFFFFFFFF;
                // set to 0 the bytes that are not used for addressing
                for (size_t j = 3; j >= ADDRESSING_BYTES; j--) {
                    ID.bytes[j] = 0;
                }
                // avoid ID = 0
                if (ID.value == 0) {
                    continue;
                }
                // Ensure the ID is not already in usedIDs
                int isUnique = 1;
                for (size_t i = 0; i < usedIDCount; i++) {
                    if (usedIDs[i].value == ID.value) {
                        isUnique = 0;
                        break;
                    }
                }
                if (isUnique) {
                    break;
                }
            } 
            while (1);
        }

        // Remove 50% of the IDs
        size_t removeCount = usedIDCount / 2;
        for (size_t i = 0; i < removeCount; i++) {
            size_t randomIndex = rand() % usedIDCount;
            while (usedIDs[randomIndex].value == 0) {
                randomIndex = rand() % usedIDCount;
            }

            if (removeFile(usedIDs[randomIndex])) {
                printf("Error removing file with ID: %x\n", usedIDs[randomIndex].value);
                free(usedIDs);
                return 1;
            }

            usedIDs[randomIndex].value = 0; // Mark as removed
        }

        // reset usedIDs and usedIDCount
        size_t oldUsedIDCount = usedIDCount;
        usedIDCount = 0;
        for (size_t i = 0; i < oldUsedIDCount; i++) {
            if(usedIDs[i].value != 0) {
                usedIDs[usedIDCount++] = usedIDs[i];
            }
        }
    }

    // Add random files until the disk gets full
    while (1) {
        int randomIndex = rand() % 5;

        if (addFile(filePaths[randomIndex], ID)) {
            // disk is full
            break;
        }

        usedIDs[usedIDCount++] = ID; // Store the used ID
        // Generate a new unique ID
        do {
            ID.value = rand() % 0xFFFFFFFF;
            // set to 0 the bytes that are not used for addressing
            for (size_t j = 3; j >= ADDRESSING_BYTES; j--) {
                ID.bytes[j] = 0;
            }
            // avoid ID = 0
            if (ID.value == 0) {
                continue;
            }
            // Ensure the ID is not already in usedIDs
            int isUnique = 1;
            for (size_t i = 0; i < usedIDCount; i++) {
                if (usedIDs[i].value == ID.value) {
                    isUnique = 0;
                    break;
                }
            }
            if (isUnique) {
                break;
            }
        } 
        while (1);
    }

    free(usedIDs);
    return 0;
}

unsigned char fatReorganizationTest() {
    // need the disk to have already been manipulated

    if (FILE_ALLOCATION == 0) {
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
    else {
        // perform a disk defragmentation
        if (diskDefragmentation()) {
            printf("Error in disk defragmentation\n");
            return 1;
        }
        // check through any FAT entry to make sure no page is left empty between two entries
        AddressType index = {0};
        AddressType previousAddress;
        unsigned char firstEntry = 1;
        CA_FATEntry FATinfo = CA_getFATEntry(index);
        for (size_t i = FATinfo.length.value; i >= 1; i--) {
            // for each entry in the FAT
            index.value = i;
            CA_FATEntry entry = CA_getFATEntry(index);
            if (entry.page.value != previousAddress.value && !firstEntry) {
                // there is a gap between two entries
                return 1;
            }
            firstEntry = 0;
            // load the end address of this entry
            previousAddress.value = entry.page.value + entry.length.value;
        }
        return 0;
    }
}

void runTests() {
    printf("Running tests with:\n");
    printf("File number: %zu\n", (size_t)TEST_FILE_NBR);
    if (TEST_FILE_SIZE < 1024) {
        printf("File size: %zu B\n\n", (size_t)TEST_FILE_SIZE);
    } else if (TEST_FILE_SIZE < 1024 * 1024) {
        printf("File size: %zu kB\n\n", (size_t)(TEST_FILE_SIZE / 1024));
    } else {
        printf("File size: %zu MB\n", (size_t)(TEST_FILE_SIZE / (1024 * 1024)));
    }
    printf("Number of iterations for multiple write/erase test: %zu\n\n", (size_t)MAX_MIN_ITR);

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

    if (multipleWriteEraseTest(MAX_MIN_ITR)) {
        printf("Error in Multiple Write Erase test\n");
        return;
    }
    else {
        printf("Multiple Write Erase test succeeded\n");
    }

    if (fatReorganizationTest()) {
        if (FILE_ALLOCATION == 0) {
            printf("Error in FAT reorganization test\n");
        }
        else {
            printf("Error in disk defragmentation test\n");
        }
        return;
    }
    else {
        if (FILE_ALLOCATION == 0) {
            printf("FAT reorganization test succeeded\n");
        }
        else {
            printf("Disk defragmentation test succeeded\n");
        }
    }

    printf("\nAll tests passed\n\n");
}

