#include "disk.h"
#include "constants.h"
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

    // initiate the bitmap by setting '1' on the bitmap address + 1st FAT table
    // only done if bitmap and FAT are used
    size_t bitmapSize = 0;
    if (FILE_ALLOCATION == 0) {
        bitmapSize = (BITMAP_PAGES + 1) / 8 + 1; 
        unsigned char* bitmap = calloc(bitmapSize, sizeof(unsigned char));
        size_t index = 0;
        size_t bit = 0;
        size_t bitsLeft = BITMAP_PAGES + 1;
        while(bitsLeft--) {
            *(bitmap+index) = *(bitmap+index) | (0x80 >> bit);
            if (++bit == 8) {
                index += 1;
                bit = 0;
            }
        }
        write(disk, bitmap, bitmapSize);
        free(bitmap);
    }

    //Writing 1024 '0'-bytes at a time
    unsigned char buffer[1024] = {0};
    size_t leftDim = dimensions - bitmapSize;    // number of bytes left
    size_t itrDim;                  // number of bytes written at this itr

    while (leftDim > 0) {
        itrDim = (leftDim > 1024) ? 1024 : leftDim;
        write(disk, buffer, itrDim);
        leftDim -= itrDim;
    }

    close(disk);
}

void diskWrite(size_t diskPos, unsigned char* data, size_t len) {
    size_t diskSize = getDiskSize();

    if (diskPos + len <= diskSize) {
        int disk = open("disk", O_WRONLY | O_CREAT, 0644);
        lseek(disk, diskPos, SEEK_SET);
        // SEEK_SET specifies that diskPos is the absolute index
        write(disk, data, len);
        close(disk);
    }
    else {
        printf("Error: Attempt to write beyond disk size\n");
    }
}

void diskRead(size_t diskPos, unsigned char* mem, size_t memPos, size_t len) {
    size_t diskSize = getDiskSize();
    if (diskPos + len <= diskSize) {
        int disk = open("disk", O_RDONLY, 0644);
        lseek(disk, diskPos, SEEK_SET);
        size_t memPtr = (size_t)mem + memPos;
        read(disk, (void*)memPtr, len);
        close(disk);
    }
}
