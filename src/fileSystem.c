#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "fileSystem.h"
#include "disk.h"

void getPage(unsigned char pos, unsigned char* buffer) {
    // with a buffer of 16 bytes
    diskRead(pos*16, buffer, 0, 16);
}

void setPage(unsigned char pos, unsigned char* buffer) {
    // with a buffer of 16 bytes
    diskWrite(pos*16, buffer, 16);
}

unsigned char getFreePage() {
    // far from optimal due to random page selection
    // implies a variable execution time
    srand(time(NULL));
        // random seed
    unsigned char* bitmap = malloc(32);
    diskRead(0, bitmap, 0, 32);

    char full = 1;
    for (size_t i = 0; i < 32; i++) {
        if (bitmap[i] != (unsigned char)0xFF) {
            full = 0;
            break;
        }
    }

    if (full) {
        free(bitmap);
        perror("Disk full");
    }

    unsigned char page = 0;
    while(0b1 << (7-(page%8)) & bitmap[page/8]) {
        // page already used
        page = (unsigned char)rand() % 255;
    }
    free(bitmap);

    return page;
}

void updateBitmap(unsigned char page) {
    unsigned char* bitmap = malloc(32);
    diskRead(0, bitmap, 0, 32);
    bitmap[page/8] ^= 0b1 << (7-(page%8));
    // XORing to toggle the bit
    diskWrite(0, bitmap, 32);
    free(bitmap);
}

void addToFAT(unsigned char ID, unsigned char page) {
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
        // 2 because pages 0 and 1 are reserved for the bitmap
    getPage(pageIndex, fat);
    while (*fat == 0xFF) {
        for (char i = 1; i < 8; i++) {
            if (*(fat + i*2) == 0x00) {
                *(fat + i*2) = ID;
                *(fat + i*2 + 1) = page;
                setPage(pageIndex, fat);
                free(fat);
                return;
            }
        }
        if (*(fat + 1) == 0xFF) {
            // it was the last FAT page
            break;
        }
        pageIndex = *(fat + 1);
        getPage(pageIndex, fat);
    }
    // all FAT pages are full -> create a new one
    
    unsigned char newPageIndex = getFreePage();
    updateBitmap(newPageIndex);
    *(fat + 1) = newPageIndex;
    // stores the modified FAT page
    setPage(pageIndex, fat);
    
    for (size_t i = 0; i < 16; i++) {
        if (i < 2) {
            fat[i] = 0xFF;
            // 1st byte to 0xFF as it is a FAT page
            // 2nd byte to 0xFF as it is the last FAT page
        } 
        // adding the new file to the new FAT page
        else if (i == 2) {
            fat[i] = ID;
        }
        else if (i == 3) {
            fat[i] = page;
        }
        // the rest of the page is empty
        else {
            fat[i] = 0x00;
        }
    }
    setPage(newPageIndex, fat);

    free(fat);
}

unsigned char removeFromFat(unsigned char ID) {

    // TODO : remove FAT page if it was the last entry
    // TODO : reorganize FAT pages

    // remove the file from the FAT and return the first page address
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
    getPage(pageIndex, fat);
    while (*fat == 0xFF) {
        for (char i = 1; i < 8; i++) {
            if (*(fat + i*2) == ID) {
                *(fat + i*2) = 0x00;
                // no need to remove previous page
                unsigned char ret = *(fat + i*2 + 1);
                setPage(pageIndex, fat);
                free(fat);
                return ret;
            }
        }
        pageIndex = *(fat + 1);
        getPage(pageIndex, fat);
    }
    free(fat);
    perror("Trying to remove a non-existing file");
    return 255;
}

void reorganizeFAT() {
    // TODO
}

unsigned char searchFAT(unsigned char ID) {
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
        // 2 because pages 0 and 1 are reserved for the bitmap
    unsigned char page = 0;
    getPage(pageIndex, fat);
    while (*fat == 0xFF) {
        for (char i = 1; i < 8; i++) {
            if (*(fat + i*2) == ID) {
                page = *(fat + i*2 + 1);
                break;
            }
        }
        if (page != 0) {
            break;
        }
        pageIndex = *(fat + 1);
        getPage(pageIndex, fat);
    }
    if (page == 0) {
        perror("File not found");
    }

    free(fat);
    return page;
}

void addFile(const char* filePath, unsigned char ID) {

    // compute file size
    int file = open(filePath, O_RDONLY, 0644);
    struct stat fileStat;
    fstat(file, &fileStat);
    size_t fileSize = fileStat.st_size;

    // file setup
    lseek(file, 0, SEEK_SET);

    // number of pages needed
    size_t pagesNbr = fileSize/15;
        // 15 as pages are 15 bytes long with 1 byte pointing to the next page

    unsigned char* buffer = malloc(16);
    unsigned char page = getFreePage();
    updateBitmap(page);
    addToFAT(ID, page);
    unsigned char nextPage;

    while(pagesNbr > 1) {
        unsigned char nextPage = getFreePage();
        updateBitmap(nextPage);
        buffer[0] = nextPage;
        read(file, buffer + 1, 15);
        diskWrite(page*16, buffer, 16);
        page = nextPage;
        pagesNbr--;
    }

    if (pagesNbr == 1) {
        nextPage = 0x00;
        buffer[0] = nextPage;
        read(file, buffer + 1, 15);
        diskWrite(page*16, buffer, 16);
    }
    free(buffer);
}

void loadFile(unsigned char ID, unsigned char* mem, size_t len) {
    
    // Step 1: get the number of pages
    unsigned char firstPageIndex = searchFAT(ID);
    size_t pagesNbr = getFileSize(ID);
    if (len < pagesNbr*15) {
        perror("Memory too small");
    }

    // Step 2: load the file
    unsigned char* pageBuffer = malloc(16);
    unsigned char* memPtr = mem;
    unsigned char pageIndex = firstPageIndex;
    for (size_t i = 0; i < pagesNbr; i++) {
        if (pageIndex == 0x00) {
            perror("File corrupted");
        }
        getPage(pageIndex, pageBuffer);
        for (size_t j = 1; j < 16; j++) {
            *(memPtr++) = pageBuffer[j];
        }
        pageIndex = pageBuffer[0];
    }
}

void removeFile(unsigned char ID) {
    unsigned char index = removeFromFat(ID);
    unsigned char* buffer = malloc(16);
    do {
        getPage(index, buffer);
        updateBitmap(index);
        index = *buffer;
    }
    while(*buffer != 0x00);
}

size_t getFileSize(unsigned char ID) {
    unsigned char firstPageIndex = searchFAT(ID);
    unsigned char* buffer = malloc(1);
    *(buffer) = firstPageIndex;
    size_t pagesNbr = 1;
    diskRead(*(buffer)*16, buffer, 0, 1);
    while (*(buffer) != 0x00) {
        diskRead(*(buffer)*16, buffer, 0, 1);
        pagesNbr++;
    }
    free(buffer);
    return pagesNbr;
}

