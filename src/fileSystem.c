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
    bitmap[page/8] |= 0b1 << (7-(page%8));
    diskWrite(0, bitmap, 32);
    free(bitmap);
}

void addToFAT(unsigned char ID, unsigned char page) {
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
        // 2 because pages 0 and 1 are reserved for the bitmap
    getPage(pageIndex, fat);
    char written = 0;
    while (*(unsigned short*)fat == 0xFFFF) {
        for (char i = 1; i < 8; i++) {
            if (*(fat + i*2) == 0x00) {
                *(fat + i*2) = ID;
                *(fat + i*2 + 1) = page;
                setPage(pageIndex, fat);
                written = 1;
                break;
            }
        }
        if (written) {
            break;
        }
        pageIndex++;
        getPage(pageIndex, fat);
    }
    // all FAT pages are full -> create a new one
    
    // TODO later
    // reported as it would need to eventually move a page that would happen to be next to the last FAT page

    free(fat);
    
}

unsigned char searchFAT(unsigned char ID) {
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
        // 2 because pages 0 and 1 are reserved for the bitmap
    unsigned char page = 0;
    getPage(pageIndex, fat);
    while (*(unsigned short*)fat == 0xFFFF) {
        for (char i = 1; i < 8; i++) {
            if (*(fat + i*2) == ID) {
                page = *(fat + i*2 + 1);
                break;
            }
        }
        if (page != 0) {
            break;
        }
        pageIndex++;
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
    unsigned char* buffer = malloc(1);
    *(buffer) = firstPageIndex;
    unsigned char pagesNbr = 1;
    diskRead(*(buffer)*16, buffer, 0, 1);
    while (*(buffer) != 0x00) {
        diskRead(*(buffer)*16, buffer, 0, 1);
        pagesNbr++;
    }
    free(buffer);
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

