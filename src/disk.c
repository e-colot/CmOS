#include "disk.h"
#include <sys/stat.h> // For fstat()
#include <fcntl.h>    // For open()
#include <unistd.h>   // For write() and close()
#include <stdio.h>
#include <stdlib.h>

// getDiskSize not in disk.h as it is only used in this file
size_t getDiskSize() {
    static size_t diskSize = 0;
    if (diskSize == 0) {
        int disk = open("disk", O_RDONLY, 0644);
        struct stat fileStat;
        fstat(disk, &fileStat);
        diskSize = fileStat.st_size;
        close(disk);
    }
    return diskSize;
}

void diskInit(size_t dimensions) {
    /*Clears the disk and fill it with only 0 for the length of dimensions*/
    int disk = open("disk", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    //Writing 1024 bytes at a time
    char buffer[1024] = {0};
    size_t leftDim = dimensions;    // number of bytes left
    size_t itrDim;                  // number of bytes written at this itr
    while (leftDim > 0) {
        itrDim = (leftDim > 1024) ? 1024 : leftDim;
        write(disk, buffer, itrDim);
        leftDim -= itrDim;
    }
    close(disk);
}

void diskWrite(size_t diskPos, char* data, size_t len) {
    size_t diskSize = getDiskSize();

    if (diskPos + len <= diskSize) {
        int disk = open("disk", O_WRONLY | O_CREAT, 0644);
        lseek(disk, diskPos, SEEK_SET);
        // SEEK_SET specifies that diskPos is the absolute index
        write(disk, data, len);
        close(disk);
    }
}

void diskRead(size_t diskPos, char* mem, size_t memPos, size_t len) {
    size_t diskSize = getDiskSize();
    if (diskPos + len <= diskSize) {
        int disk = open("disk", O_RDONLY, 0644);
        lseek(disk, diskPos, SEEK_SET);
        size_t memPtr = (size_t)mem + memPos;
        read(disk, (void*)memPtr, len);
        close(disk);
    }
}

void storeProgram(const char* filePath, size_t pos) {
    // ai generated code
    // Open the file
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to read the file
    char* buffer = (char*)malloc(fileSize);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }

    // Read the file into the buffer
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Write the buffer to the disk
    diskWrite(pos, buffer, fileSize);

    // Free the allocated memory
    free(buffer);
}