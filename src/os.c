#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "os.h"
#include "disk.h"

void getPage(unsigned char pos, char* buffer) {
    // with a buffer of 16 bytes
    diskRead(pos*16, buffer, 0, 16);
}

void setPage(unsigned char pos, char* buffer) {
    // with a buffer of 16 bytes
    diskWrite(pos*16, buffer, 16);
}

unsigned char getFreePage() {
    // far from optimal due to random page selection
    // implies a variable execution time
    srand(time(NULL));
        // random seed
    char* bitmap = malloc(32);
    diskRead(0, bitmap, 0, 32);

    char full = 1;
    for (size_t i = 0; i < 32; i++) {
        if (bitmap[i] != (char)0xFF) {
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
    // DEBUG
    printf("Page %d\n", page);
    return page;
}

void updateBitmap(unsigned char page) {
    char* bitmap = malloc(32);
    diskRead(0, bitmap, 0, 32);
    bitmap[page/8] |= 0b1 << (7-(page%8));
    diskWrite(0, bitmap, 32);
    free(bitmap);
}

void addToFAT(unsigned char ID, unsigned char page) {
    char* fat = malloc(16);
    unsigned char pageIndex = 2;
    getPage(pageIndex, fat);
    char written = 0;
        // 2 because pages 0 and 1 are reserved for the bitmap
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

void addFile(const char* filePath, unsigned char ID) {

    // compute file size
    int file = open(filePath, O_RDONLY, 0644);
    struct stat fileStat;
    fstat(file, &fileStat);
    size_t fileSize = fileStat.st_size;

    // file setup
    lseek(file, 0, SEEK_SET);

    // number of pages needed
    size_t pages = fileSize/15;
        // 15 as pages are 15 bytes long with 1 byte pointing to the next page

    char* buffer = malloc(16);
    unsigned char page = getFreePage();
    updateBitmap(page);
    addToFAT(ID, page--);
    unsigned char nextPage;

    while(pages > 1) {
        unsigned char nextPage = getFreePage();
        updateBitmap(nextPage);
        buffer[0] = nextPage;
        read(file, buffer + 1, 15);
        diskWrite(page*16, buffer, 16);
        page = nextPage;
        pages--;
    }

    if (pages == 1) {
        nextPage = 0x00;
        buffer[0] = nextPage;
        read(file, buffer + 1, 15);
        diskWrite(page*16, buffer, 16);
    }
    free(buffer);
}

