#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "fileSystem.h"
#include "constants.h"
#include "disk.h"

// --------------- ADRESSING OPERATIONS ----------------
AddressType getAddress(unsigned char* src) {
    // returns the address from src
    AddressType srcAddress;
    // needed to set every byte of srcAddress.bytes to 0
    srcAddress.value = 0;
    for (unsigned char i = 0; i < ADDRESSING_BYTES; i++) {
        srcAddress.bytes[i] = *src;
        src++;
    }
    return srcAddress;
}
void setAddress(unsigned char* dest, AddressType src) {
    // sets the address from src to dest
    for (unsigned char i = 0; i < ADDRESSING_BYTES; i++) {
        *(unsigned char*)dest = src.bytes[i];
        dest++;
    }
}
unsigned char checkAddress(unsigned char* src, size_t value) {
    // checks if the address is equal to value
    // returns 1 if they are the same, 0 otherwise
    AddressType srcAddress = getAddress(src);
    return srcAddress.value == value;
}
// --------------- END ADRESSING OPERATIONS --------------

void getPage(AddressType pos, unsigned char* buffer) {
    // with a buffer of PAGE_SIZE bytes
    diskRead(pos.value*PAGE_SIZE, buffer, 0, PAGE_SIZE);
}

void setPage(AddressType pos, unsigned char* buffer) {
    // with a buffer of PAGE_SIZE bytes
    diskWrite(pos.value*PAGE_SIZE, buffer, PAGE_SIZE);
}

AddressType getFreePage() {
    // get a free page in the bitmap
    // NOT in charge of modifying the bitmap

    // far from optimal due to random page selection
    // implies a variable execution time
    static int seedInitialized = 0;
    if (!seedInitialized) {
        srand(time(NULL));
        seedInitialized = 1;
    }
    unsigned char* bitmap = malloc(BITMAP_SIZE);
    diskRead(0, bitmap, 0, BITMAP_SIZE);

    char full = 1;
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
        if (bitmap[i] != (unsigned char)0xFF) {
            full = 0;
            break;
        }
    }

    if (full) {
        free(bitmap);
        perror("Disk full");
        AddressType zero;
        zero.value = 0;
        return zero;
    }

    AddressType page;
    page.value = (size_t)rand() % ((DISK_SIZE/PAGE_SIZE)-1);
    while(0b1 << (7-(page.value%8)) & bitmap[page.value/8]) {
        // page already used
        page.value = (size_t)rand() % ((DISK_SIZE/PAGE_SIZE)-1);
    }
    free(bitmap);
    return page;
}

void updateBitmap(AddressType page) {
    unsigned char* bitmap = malloc(BITMAP_SIZE);
    diskRead(0, bitmap, 0, BITMAP_SIZE);
    bitmap[page.value/8] ^= 0b1 << (7-(page.value%8));
    // XORing to toggle the bit
    diskWrite(0, bitmap, BITMAP_SIZE);
    free(bitmap);
}

void addToFAT(AddressType ID, AddressType page) {
    // adds the entry [ID, page] to the FAT
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    *(fat + ADDRESSING_BYTES) = FAT_START; 
        // set as next fat page address of the first FAT page
    while (*(fat + ADDRESSING_BYTES)) {
        // interpret it as "while the next FAT page is not 0", 
        // which would indicate it was the last FAT page

        // Copy ADDRESSING_BYTES bytes from fat+ADDRESSING_BYTES to pageIndex
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
        getPage(pageIndex, fat);

        for (size_t i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
            if (checkAddress(fat + i*ADDRESSING_BYTES, 0)) {
                // if the entry is empty, put the ID and page in it
                setAddress(fat + i*ADDRESSING_BYTES, ID);
                setAddress(fat + i*ADDRESSING_BYTES + ADDRESSING_BYTES, page);
                setPage(pageIndex, fat);
                free(fat);
                return;
            }
        }
    }

    // all FAT pages are full -> create a new one
    AddressType newPageIndex = getFreePage();
    updateBitmap(newPageIndex);
    setAddress(fat + ADDRESSING_BYTES, newPageIndex);
    // stores the modified FAT page
    setPage(pageIndex, fat);
    
    setAddress(fat, page); // last FAT page (reverse linking)
    // set next FAT page address as zero (to indicate it is the last one)
    AddressType zero;
    zero.value = 0x00;
    setAddress(fat + ADDRESSING_BYTES, zero);
    // set the ID and page in the new FAT page
    setAddress(fat + 2*ADDRESSING_BYTES, ID);
    setAddress(fat + 3*ADDRESSING_BYTES, page);

    // set the rest of the page to 0
    memset(fat + 4, 0, PAGE_SIZE - 4);
    setPage(newPageIndex, fat);

    free(fat);
}

void removeFATPage(AddressType pageIndex) {
    // a bit overkill as it could handle a page in the middle being deleted even though it will
    // only be called in the case of the last FAT page being deleted (if implemented as planned)
    // it still will not remove the first FAT page (located at pageIndex = 2)
    if (pageIndex.value == FAT_START) {
        // by convention, the first FAT page is at address FAT_START -> cannot be deleted
        return;
    }
    unsigned char* fat = malloc(PAGE_SIZE);
    getPage(pageIndex, fat);

    // store the first two addresses in FAT as they link to the previous and next FAT pages
    AddressType previousIndex = getAddress(fat);
    AddressType nextIndex = getAddress(fat + ADDRESSING_BYTES);

    // checks the page is empty
    for (size_t i = 2; i <= PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
        if (!checkAddress((fat + i*ADDRESSING_BYTES), 0)) {
            // the i-th entry is not empty
            free(fat);
            return;
        }
    }

    if (previousIndex.value == 0) {
        // if it was not the first FAT page
        // load the previous FAT page
        getPage(previousIndex, fat);
        // set the next FAT page address to the one after the one that is being deleted
        setAddress(fat + ADDRESSING_BYTES, nextIndex);
        setPage(previousIndex, fat);
    }
    if (nextIndex.value != 0) {
        // if it was not the last FAT page
        // load the next FAT page
        getPage(nextIndex, fat);
        // set the previous FAT page address to the one before the one that is being deleted
        setAddress(fat, previousIndex);
        setPage(nextIndex, fat);
    }
    updateBitmap(pageIndex);
    free(fat);
}

void reorganizeFAT(AddressType pageIndex) {
    // It is known that the current page has a "hole" in it and it needs to be filled

    // There is no benefit from moving the hole to the end of the page:
    //
    //    used        used
    //    empty       used
    //    used        empty
    //
    // here above the second columns is not better than the first one
    // The only thing to do is to get a used line from the last FAT page to fill the hole.
    // And if this is the last FAT page, just check if it is not empty
    unsigned char* fat = malloc(PAGE_SIZE);
    getPage(pageIndex, fat);

    unsigned char emptySpot = PAGE_SIZE + 1;
    // REMARK: emptySpot takes into account ADDRESSING_BYTES, DO NOT multiply it further
    for (size_t i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
        if (checkAddress((fat + i*ADDRESSING_BYTES), 0)) {
            // the i-th entry is empty
            emptySpot = ADDRESSING_BYTES*i;
            break;
        }
    }
    if (emptySpot == PAGE_SIZE + 1) {
        // no empty spot found
        perror("Called reorganizeFAT for a full page");
        return;
    }


    if (checkAddress((fat + emptySpot), 0)) {
        // if not last page
        AddressType lastPage;
        unsigned char* lastFAT = malloc(PAGE_SIZE);
        *(lastFAT + 1) = *(fat + 1);
        while (checkAddress((lastFAT + ADDRESSING_BYTES), 0)) {
            lastPage = getAddress(lastFAT + ADDRESSING_BYTES);
            getPage(lastPage, lastFAT);
        }
        // lastFAT contains the last FAT page
        AddressType tmpID, tmpPage;
        tmpID.value = 0;
        tmpPage.value = 0;
        for (unsigned char i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
            if (!checkAddress((lastFAT + i*ADDRESSING_BYTES), 0)) {
                // the i-th entry is not empty
                // copy the ID and page to be moved
                tmpID = getAddress(lastFAT + i*ADDRESSING_BYTES);
                tmpPage = getAddress(lastFAT + i*ADDRESSING_BYTES + ADDRESSING_BYTES);

                // remove the entry from the last FAT page
                AddressType zero;
                zero.value = 0;
                setAddress(lastFAT + i*ADDRESSING_BYTES, zero);

                setPage(lastPage, lastFAT);
                break;
            }
        }
        if (tmpPage.value == 0) {
            // should never happens, would mean that the last page was already empty
            perror("Last FAT page was already empty and was still linked");
            return;
        }
        // still have to put "tmpID" and "tmpPage" in "emptySpot"
        // and check if lastFAT is empty now
        setAddress(fat + emptySpot, tmpID);
        setAddress(fat + emptySpot + ADDRESSING_BYTES, tmpPage);
        // write back the modified FAT page in disk
        setPage(pageIndex, fat);
        // lastFAT will not be freed, it will replace fat and then it will follow with the case
        // of fat being the last FAT page
        free(fat);
        fat = lastFAT;
        pageIndex = lastPage;
    }
    // checks if page is empty
    char empty = 1;
    for (size_t i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
        if (!checkAddress((fat + i*ADDRESSING_BYTES), 0)) {
            // the i-th entry is not empty
            empty = 0;
            break;
        }
    }
    if (empty) {
        removeFATPage(pageIndex);
    }
    free(fat);
}

AddressType removeFromFat(AddressType ID) {

    // remove the file from the FAT and return the first page address
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (size_t i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
            if (checkAddress((fat + i*ADDRESSING_BYTES), ID.value)) {
                // the i-th entry corresponds to the ID to be removed
                AddressType zero;
                zero.value = 0x00;
                setAddress(fat + i*ADDRESSING_BYTES, zero);

                // copy the page address to be returned
                AddressType ret = getAddress(fat + i*ADDRESSING_BYTES + ADDRESSING_BYTES);

                setPage(pageIndex, fat);
                free(fat);
                reorganizeFAT(pageIndex); // will remove the FAT page if it is empty
                return ret;
            }
        }
        // updates the page index to the next FAT page
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
    }
    free(fat);
    perror("Trying to remove a non-existing file");
    AddressType zero;
    zero.value = 0;
    return zero;
}

AddressType searchFAT(AddressType ID) {
    // returns the page at which the file is stored
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    
    AddressType page;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (size_t i = 2; i < PAGE_SIZE/(2*ADDRESSING_BYTES); i++) {
            if (checkAddress((fat + i*ADDRESSING_BYTES), ID.value)) {
                // the i-th entry corresponds to the ID to be found
                // copy the page address to be returned
                page = getAddress(fat + i*ADDRESSING_BYTES + ADDRESSING_BYTES);
                free(fat);
                return page;
            }
        }
        // updates the page index to the next FAT page
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
    }
    perror("File not found");
    free(fat);
    page.value = 0;
    return page;
}

size_t getFileSize(AddressType ID) {
    AddressType pageIndex = searchFAT(ID);
    if (pageIndex.value == 0) {
        // file not found
        return 0;
    }

    size_t pagesNbr = 1;

    diskRead(pageIndex.value*PAGE_SIZE, (unsigned char*)&pageIndex, 0, ADDRESSING_BYTES);
    while (pageIndex.value != 0x00) {
        diskRead(pageIndex.value*PAGE_SIZE, (unsigned char*)&pageIndex, 0, ADDRESSING_BYTES);
        pagesNbr++;
    }
    return pagesNbr;
}

void addFile(const char* filePath, AddressType ID) {

    // compute file size
    int file = open(filePath, O_RDONLY, 0644);
    struct stat fileStat;
    fstat(file, &fileStat);
    size_t fileSize = fileStat.st_size;

    // file setup
    lseek(file, 0, SEEK_SET);

    // number of pages needed
    size_t pagesNbr = fileSize/(PAGE_SIZE - 1);
        // 15 as pages are 15 bytes long with 1 byte pointing to the next page

    unsigned char* buffer = malloc(PAGE_SIZE);
    AddressType page = getFreePage();

    updateBitmap(page);
    addToFAT(ID, page);

    AddressType nextPage;

    while(pagesNbr > 1) {
        nextPage = getFreePage();
        // add the used page to the bitmap
        updateBitmap(nextPage);
        // add a link to the next page in the current page
        setAddress(buffer, nextPage);
        read(file, buffer + ADDRESSING_BYTES, PAGE_SIZE - ADDRESSING_BYTES);
        diskWrite(page.value*PAGE_SIZE, buffer, PAGE_SIZE);
        page = nextPage;
        pagesNbr--;
    }

    if (pagesNbr == 1) {
        // link to 0 to indicate the end of the file
        nextPage.value = 0;
        setAddress(buffer, nextPage);
        read(file, buffer + ADDRESSING_BYTES, PAGE_SIZE - ADDRESSING_BYTES);
        diskWrite(page.value*PAGE_SIZE, buffer, PAGE_SIZE);
    }
    free(buffer);
}

void loadFile(AddressType ID, unsigned char* mem, size_t len) {
    
    // Step 1: get the number of pages
    AddressType firstPageIndex = searchFAT(ID);
    if (firstPageIndex.value == 0) {
        // file not found
        return;
    }
    size_t pagesNbr = getFileSize(ID);
    if (len < pagesNbr*(PAGE_SIZE - 1)) {
        perror("Memory too small");
    }

    // Step 2: load the file
    unsigned char* pageBuffer = malloc(PAGE_SIZE);
    unsigned char* memPtr = mem;
    AddressType pageIndex = firstPageIndex;
    for (size_t i = 0; i < pagesNbr; i++) {
        if (pageIndex.value == 0x00) {
            // impossible to find the next page
            perror("File corrupted");
        }
        getPage(pageIndex, pageBuffer);
        for (unsigned char j = 1; j < PAGE_SIZE; j++) {
            *(memPtr++) = pageBuffer[j];
        }
        pageIndex = getAddress(pageBuffer);
    }
}

void removeFile(AddressType ID) {
    AddressType index = removeFromFat(ID);
    if (index.value == 0) {
        // file not found in FAT
        return;
    }
    unsigned char* buffer = malloc(PAGE_SIZE);
    do {
        getPage(index, buffer);
        updateBitmap(index);
        index = getAddress(buffer);
    }
    while(*buffer != 0x00);
}

