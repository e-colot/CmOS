#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

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
    for (unsigned char i = 0; i < 32; i++) {
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
    unsigned char pageIndex;
    *(fat + 1) = 2; 
        // will be then attributed to pageIndex
        // 2 because pages 0 and 1 are reserved for the bitmap
    while (*(fat + 1)) {
        // interpret it as "while the next FAT page is not 0", 
        // which would indicate it was the last FAT page
        pageIndex = *(fat + 1);
        getPage(pageIndex, fat);
        for (unsigned char i = 1; i < 8; i++) {
            if (*(fat + i*2) == 0x00) {
                *(fat + i*2) = ID;
                *(fat + i*2 + 1) = page;
                setPage(pageIndex, fat);
                free(fat);
                return;
            }
        }
    }

    // all FAT pages are full -> create a new one
    unsigned char newPageIndex = getFreePage();
    updateBitmap(newPageIndex);
    *(fat + 1) = newPageIndex;
    // stores the modified FAT page
    setPage(pageIndex, fat);
    
    fat[0] = pageIndex; // reverse link
    fat[1] = 0x00; // last FAT page
    fat[2] = ID;
    fat[3] = page;
    memset(fat + 4, 0, 12);
    setPage(newPageIndex, fat);

    free(fat);
}

void removeFATPage(unsigned char pageIndex) {
    // a bit overkill as it could handle a page in the middle being deleted
    // it still will not remove the first FAT page (located at pageIndex = 2)
    if (pageIndex == 2) {
        // by convention, the first FAT page is at address 2 -> cannot be deleted
        return;
    }
    unsigned char* fat = malloc(16);
    getPage(pageIndex, fat);
    unsigned char previousIndex = *fat;
    unsigned char nextIndex = *(fat + 1);

    // checks the page is empty
    if (*(fat + 2) || *(fat + 4) || *(fat + 6) || *(fat + 8) || *(fat + 10) || *(fat + 12) || *(fat + 14)) {
        perror("Trying to remove a non-empty FAT page");
        free(fat);
        return;
    }

    if (previousIndex) {
        // if it was not the first FAT page
        getPage(previousIndex, fat);
        *(fat + 1) = nextIndex;
        setPage(previousIndex, fat);
    }
    if (nextIndex) {
        // if it was not the last FAT page
        getPage(nextIndex, fat);
        *(fat) = previousIndex;
        setPage(nextIndex, fat);
    }
    updateBitmap(pageIndex);
    free(fat);
}

void reorganizeFAT(unsigned char pageIndex) {
    // It is known that the current page has a "hole" in it and it needs to be filled
    // There is no benefit from moving the hole to the end of the page:
    //
    //    used        used
    //    empty       used
    //    used        empty
    //
    // here above the seconde columns is not better than the first one
    // The only thing to do is to get a used line from the last FAT page to fill the hole.
    // And if this is the last FAT page, just check if it is not empty
    unsigned char* fat = malloc(16);
    getPage(pageIndex, fat);

    unsigned char emptySpot = 17;
    for (unsigned char i = 1; i < 8; i++) {
        if (*(fat + 2*i) == 0) {
            emptySpot = 2*i;
            break;
        }
    }
    if (emptySpot == 17) {
        // no empty spot found
        perror("Called reorganizeFAT for a full page");
        return;
    }


    if (*(fat + 1)) {
        // if not last page
        unsigned char lastPage;
        unsigned char* lastFAT = malloc(16);
        *(lastFAT + 1) = *(fat + 1);
        while (*(lastFAT + 1)) {
            lastPage = *(lastFAT + 1);
            getPage(lastPage, lastFAT);
        }
        // lastFAT contains the last FAT page
        unsigned short replacement = 0;
        for (unsigned char i = 1; i < 8; i++) {
            if (*(lastFAT + 2*i) != 0) {
                replacement = *((unsigned short*) (lastFAT + 2*i));
                *(lastFAT + 2*i) = 0x00;
                setPage(lastPage, lastFAT);
                break;
            }
        }
        if (replacement == 0) {
            // should never happens, would mean that the last page is empty
            perror("Last FAT page is empty and not deleted");
            return;
        }
        // still have to put "replacement" in "emptySpot"
        // and check if lastFAT is empty now
        *((unsigned short*)(fat + emptySpot)) = replacement;
        setPage(pageIndex, fat);
        // lastFAT will not be freed, it will replace fat and then it will follow with the case
        // of fat being the last FAT page
        free(fat);
        fat = lastFAT;
        pageIndex = lastPage;
    }
    // checks if page is empty
    char empty = 1;
    for (unsigned char i = 1; i < 8; i++) {
        if (*(fat + 2*i) != 0) {
            empty = 0;
            break;
        }
    }
    if (empty) {
        removeFATPage(pageIndex);
    }
    free(fat);
}

unsigned char removeFromFat(unsigned char ID) {

    // remove the file from the FAT and return the first page address
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
    while (pageIndex) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (unsigned char i = 1; i < 8; i++) {
            if (*(fat + i*2) == ID) {
                *(fat + i*2) = 0x00;
                unsigned char ret = *(fat + i*2 + 1);
                setPage(pageIndex, fat);
                free(fat);
                reorganizeFAT(pageIndex); // will remove the FAT page if it is empty
                return ret;
            }
        }
        pageIndex = *(fat + 1);
    }
    free(fat);
    perror("Trying to remove a non-existing file");
    return 255;
}

unsigned char searchFAT(unsigned char ID) {
    unsigned char* fat = malloc(16);
    unsigned char pageIndex = 2;
        // 2 because pages 0 and 1 are reserved for the bitmap
    unsigned char page = 0;
    while (pageIndex) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (unsigned char i = 1; i < 8; i++) {
            if (*(fat + i*2) == ID) {
                page = *(fat + i*2 + 1);
                free(fat);
                return page;
            }
        }
        pageIndex = *(fat + 1);
    }
    perror("File not found");
    free(fat);
    return 255;
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
    for (unsigned char i = 0; i < pagesNbr; i++) {
        if (pageIndex == 0x00) {
            perror("File corrupted");
        }
        getPage(pageIndex, pageBuffer);
        for (unsigned char j = 1; j < 16; j++) {
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

