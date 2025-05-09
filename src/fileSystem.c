#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "fileSystem.h"
#include "contiguousAllocation.h"
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

    unsigned char* bitmap = malloc(BITMAP_SIZE);
    diskRead(0, bitmap, 0, BITMAP_SIZE);

    char full = 1;
    for (size_t i = 0; i < (DISK_SIZE / PAGE_SIZE); i++) {
        if (!(bitmap[i / 8] & (0b1 << (7 - (i % 8))))) {
            full = 0;
            break;
        }
    }

    if (full) {
        // disk full
        free(bitmap);
        AddressType zero = {0};
        return zero;
    }

    AddressType page;
    page.value = (size_t)rand() % ((DISK_SIZE/PAGE_SIZE));
    while(0b1 << (7-(page.value%8)) & bitmap[page.value/8]) {
        // page already used
        page.value = (page.value + 1) % (DISK_SIZE/PAGE_SIZE);
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

size_t addToFAT(AddressType ID, AddressType page) {
    // adds the entry [ID, page] to the FAT
    // takes care of updating the bitmap
    // return 1 if an error occurs, 0 otherwise

    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;

    // because of the loop, fat behvaves as the previous FAT page
    // This means it needs to have as second address a pointer to the next FAT page
    pageIndex.value = FAT_START;
    setAddress(fat + ADDRESSING_BYTES, pageIndex);


    while (getAddress(fat + ADDRESSING_BYTES).value) {
        // interpret it as "while the next FAT page is not 0", 
        // which would indicate it was the last FAT page

        // Copy ADDRESSING_BYTES bytes from fat+ADDRESSING_BYTES to pageIndex
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
        getPage(pageIndex, fat);

        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
            if (checkAddress(fat + i*ADDRESSING_BYTES, 0)) {
                // if the entry is empty, put the ID and page in it
                setAddress(fat + i*ADDRESSING_BYTES, ID);
                setAddress(fat + i*ADDRESSING_BYTES + ADDRESSING_BYTES, page);
                setPage(pageIndex, fat);
                free(fat);
                return 0;
            }
        }
    }

    // all FAT pages are full -> create a new one
    AddressType newPageIndex = getFreePage();
    if (newPageIndex.value == 0) {
        // no free page available
        return 1;
    }
    updateBitmap(newPageIndex);
    setAddress(fat + ADDRESSING_BYTES, newPageIndex);
    // stores the modified FAT page
    setPage(pageIndex, fat);
    
    setAddress(fat, pageIndex); // last FAT page (reverse linking)
    // set next FAT page address as zero (to indicate it is the last one)
    AddressType zero = {0};
    setAddress(fat + ADDRESSING_BYTES, zero);
    // set the ID and page in the new FAT page
    setAddress(fat + 2*ADDRESSING_BYTES, ID);
    setAddress(fat + 3*ADDRESSING_BYTES, page);

    // set the rest of the page to 0
    memset(fat + 4*ADDRESSING_BYTES, 0, PAGE_SIZE - 4*ADDRESSING_BYTES);
    setPage(newPageIndex, fat);

    free(fat);
    return 0;
}

void reorganizeFAT() {
    // fills holes in the FAT with entries from the end of the FAT

    // load the first and last FAT pages
    AddressType lowerPageIndex, upperPageIndex, tmp;
    lowerPageIndex.value = FAT_START;
    upperPageIndex.value = FAT_START;
    unsigned char* lowerFAT = malloc(PAGE_SIZE);
    unsigned char* upperFAT = malloc(PAGE_SIZE);
    getPage(lowerPageIndex, lowerFAT);
    getPage(upperPageIndex, upperFAT);

    while (upperPageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(upperPageIndex, upperFAT);
        // save upperPageIndex as the index of the current entry in upperFAT
        tmp = upperPageIndex;

        upperPageIndex = getAddress(upperFAT + ADDRESSING_BYTES);
    }

    // update the last FAT page address
    upperPageIndex = tmp;

    size_t lowerPageEntry = 2; // index of the current entry in the lower FAT page
    size_t upperPageEntry = 2; // index of the current entry in the upper FAT page
    
    AddressType zero = {0};

    while (lowerPageIndex.value != upperPageIndex.value) {
        // while both pointers are on a different page
        unsigned char valid = 0;
        
        // find an empty entry in the lower FAT page
        // if not found, go to the next FAT page
        for (;(lowerPageEntry+2)*ADDRESSING_BYTES <= PAGE_SIZE; lowerPageEntry=lowerPageEntry+2) {
            if (checkAddress(lowerFAT + lowerPageEntry*ADDRESSING_BYTES, 0)) {
                // the entry is empty
                valid += 1;
                break;
            }
        }

        // find a non-empty entry in the upper FAT page
        // if not found, go to the previous FAT page
        for (;(upperPageEntry+2)*ADDRESSING_BYTES <= PAGE_SIZE; upperPageEntry=upperPageEntry+2) {
            if (!checkAddress(upperFAT + upperPageEntry*ADDRESSING_BYTES, 0)) {
                // the entry is not empty
                valid += 2;
                break;
            }
        }

        // swap the two entries
        if (valid == 3) {
            // both entries are valid
            AddressType ID = getAddress(upperFAT + upperPageEntry*ADDRESSING_BYTES);
            AddressType page = getAddress(upperFAT + upperPageEntry*ADDRESSING_BYTES + ADDRESSING_BYTES);
            // update lower FAT page
            setAddress(lowerFAT + lowerPageEntry*ADDRESSING_BYTES, ID);
            setAddress(lowerFAT + lowerPageEntry*ADDRESSING_BYTES + ADDRESSING_BYTES, page);
            setPage(lowerPageIndex, lowerFAT);
            // update upper FAT page
            setAddress(upperFAT + upperPageEntry*ADDRESSING_BYTES, zero);
            setPage(upperPageIndex, upperFAT);
        }
        if (valid%2 == 0) {
            // no empty entry found in the lower FAT page
            // go to the next FAT page
            lowerPageIndex = getAddress(lowerFAT + ADDRESSING_BYTES);
            getPage(lowerPageIndex, lowerFAT);
            lowerPageEntry = 2;
        }
        else if (valid <= 1) {
            // the upper FAT page is completely empty -> might be removed
            // remove the page from the bitmap
            updateBitmap(upperPageIndex);
            // go to the previous FAT page
            upperPageIndex = getAddress(upperFAT);
            getPage(upperPageIndex, upperFAT);
            // update the next FAT page address to 0 (last FAT page)
            setAddress(upperFAT + ADDRESSING_BYTES, zero);
            setPage(upperPageIndex, upperFAT);
            upperPageEntry = 2;
        }
    }

    // put the modified pages back in the disk
    setPage(lowerPageIndex, lowerFAT);
    setPage(upperPageIndex, upperFAT);

    free(lowerFAT);
    free(upperFAT);
    return;
}

AddressType removeFromFAT(AddressType ID) {

    // a FAT reorganization is done each time PAGE_SIZE/(2*ADDRESSING_BYTES) files are removed
    static size_t count = 0;
    if (count == PAGE_SIZE/(2*ADDRESSING_BYTES)) {
        reorganizeFAT();
        count = 0;
    }
    count++;

    // remove the file from the FAT and return the first page address
    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
            if (checkAddress((fat + i*ADDRESSING_BYTES), ID.value)) {
                // the i-th entry corresponds to the ID to be removed
                AddressType zero = {0};
                setAddress(fat + i*ADDRESSING_BYTES, zero);

                // copy the page address to be returned
                AddressType ret = getAddress(fat + i*ADDRESSING_BYTES + ADDRESSING_BYTES);

                setPage(pageIndex, fat);
                free(fat);
                return ret;
            }
        }
        // updates the page index to the next FAT page
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
    }
    free(fat);
    printf("Trying to remove a non-existing file\n");
    AddressType zero = {0};
    return zero;
}

AddressType searchFAT(AddressType ID) {
    // returns the page at which the file is stored

    if (FILE_ALLOCATION == 1) {
        // file allocation is done with contiguous allocation
        CA_FATEntry entry = CA_searchFAT(ID);
        return entry.page;
    }

    unsigned char* fat = malloc(PAGE_SIZE);
    AddressType pageIndex;
    pageIndex.value = FAT_START;
    
    AddressType page;
    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
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
    free(fat);
    page.value = 0;
    return page;
}

size_t getFATsize() {

    if (FILE_ALLOCATION == 1) {
        // file allocation is done with contiguous allocation
        return CA_getFATsize();
    }

    // returns the number of entries in the FAT
    unsigned char* fat = malloc(PAGE_SIZE);

    AddressType pageIndex;
    pageIndex.value = FAT_START;
    size_t entriesNbr = 0;

    while (pageIndex.value) {
        // interpret it as "while the next FAT page is not 0",
        // which would indicate it was the last FAT page
        getPage(pageIndex, fat);
        for (size_t i = 2; (i+2)*ADDRESSING_BYTES <= PAGE_SIZE; i=i+2) {
            if (!checkAddress((fat + i*ADDRESSING_BYTES), 0)) {
                // the i-th entry is not empty
                entriesNbr++;
            }
        }
        pageIndex = getAddress(fat + ADDRESSING_BYTES);
    }

    free(fat);
    return entriesNbr;
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

size_t addFile(const char* filePath, AddressType ID) {

    if (FILE_ALLOCATION == 1) {
        // file allocation is done with contiguous allocation
        return CA_addFile(filePath, ID);
    }

    // check if there already exists a file with the same ID
    AddressType pageWithSameID = searchFAT(ID);
    if (pageWithSameID.value != 0) {
        // file already exists
        printf("File with the same ID already exists\n");
        return 1;
    }

    // compute file size
    int file = open(filePath, O_RDONLY);
    if (file < 0) {
        printf("Error opening file\n");
        return 1;
    }
    struct stat fileStat;
    if (fstat(file, &fileStat) < 0) {
        printf("Error getting file stats\n");
        close(file);
        return 1;
    }
    // file size + 1 for the null terminator
    size_t fileSize = fileStat.st_size + 1;

    // file setup
    lseek(file, 0, SEEK_SET);

    // number of pages needed
    size_t pagesNbr = (fileSize + PAGE_SIZE - ADDRESSING_BYTES - 1)/(PAGE_SIZE - ADDRESSING_BYTES);
    // -ADDRESSING_BYTES because the first bytes are used for the address of the next page

    unsigned char* buffer = malloc(PAGE_SIZE);
    AddressType page = getFreePage();
    if (page.value == 0) {
        // no free page available for the first page of the file
        free(buffer);
        close(file);
        return 1;
    }

    updateBitmap(page);
    if(addToFAT(ID, page) != 0) {
        // no free page when trying to add a FAT page
        // remove the page of the file from the bitmap
        updateBitmap(page);

        free(buffer);
        close(file);
        return 1;
    }

    // set to page to later be reattributed to previousPage
    AddressType nextPage = page;
    AddressType previousPage;

    while(pagesNbr > 1) {
        previousPage = nextPage;
        nextPage = getFreePage();
        if (nextPage.value == 0) {
            // no free page available
            close(file);
            // before removing, the last page must be marked as the last page
            getPage(previousPage, buffer);
            AddressType zero = {0};
            setAddress(buffer, zero);
            setPage(previousPage, buffer);
            free(buffer);
            removeFile(ID);
            return 1;
        }
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
        size_t bytesRead = read(file, buffer + ADDRESSING_BYTES, PAGE_SIZE - ADDRESSING_BYTES);
        if (bytesRead < PAGE_SIZE - ADDRESSING_BYTES) {
            // add a null terminator
            buffer[ADDRESSING_BYTES + bytesRead] = 0x00;
            // fill the remaining buffer with 0xFF
            memset(buffer + ADDRESSING_BYTES + bytesRead + 1, 0xFF, PAGE_SIZE - bytesRead - ADDRESSING_BYTES - 1);
        }
        else {
            printf("Error: no space available for file terminator\n");
        }
        diskWrite(page.value*PAGE_SIZE, buffer, PAGE_SIZE);
    }
    free(buffer);
    close(file);
    return 0;
}

size_t loadFile(AddressType ID, unsigned char* dest, size_t len) {

    if (FILE_ALLOCATION == 1) {
        // file allocation is done with contiguous allocation
        return CA_loadFile(ID, dest, len);
    }
    
    // Step 1: get the number of pages
    AddressType firstPageIndex = searchFAT(ID);
    if (firstPageIndex.value == 0) {
        // file not found
        printf("File not found\n");
        return 1;
    }
    size_t pagesNbr = getFileSize(ID);

    if (len < (pagesNbr-1)*(PAGE_SIZE - ADDRESSING_BYTES)) {
        printf("Memory allocated to loadFile too small\n");
        return 1;
    }

    // Step 2: load the file
    unsigned char* pageBuffer = malloc(PAGE_SIZE);
    unsigned char* destPtr = dest;
    AddressType pageIndex = firstPageIndex;
    for (size_t i = 0; i < pagesNbr-1; i++) {
        getPage(pageIndex, pageBuffer);
        for (size_t j = ADDRESSING_BYTES; j < PAGE_SIZE; j++) {
            *(destPtr++) = pageBuffer[j];
        }
        pageIndex = getAddress(pageBuffer);
    }
    // last page
    getPage(pageIndex, pageBuffer);
    unsigned char terminatorSeen = 0;
    for (size_t j = PAGE_SIZE - 1; j >= ADDRESSING_BYTES; j--) {
        if (pageBuffer[j] == 0xFF) {
            // filling bytes
            continue;
        }
        destPtr = dest + (pagesNbr-1)*(PAGE_SIZE - ADDRESSING_BYTES) + j - ADDRESSING_BYTES;
        if (terminatorSeen == 0 && pageBuffer[j] == 0x00) {
            // terminator found
            terminatorSeen = 1;
            // check for destination buffer length
            if (destPtr > dest + len + ADDRESSING_BYTES) {
                // destination buffer too small
                printf("Memory allocated to loadFile too small\n");
                free(pageBuffer);
                return 1;
            }
            continue;
        }
        if (terminatorSeen == 0) {
            // terminator not found 
            printf("Error: file terminator not found\n");
            free(pageBuffer);
            return 1;
        }
        // copy the byte to the destination buffer
        *destPtr = pageBuffer[j];
    }
    free(pageBuffer);
    return 0;
}

size_t removeFile(AddressType ID) {

    if (FILE_ALLOCATION == 1) {
        // file allocation is done with contiguous allocation
        return CA_removeFile(ID);
    }

    AddressType index = removeFromFAT(ID);
    if (index.value == 0) {
        // file not found in FAT
        return 1;
    }
    unsigned char* buffer = malloc(PAGE_SIZE);
    do {
        getPage(index, buffer);
        updateBitmap(index);
        index = getAddress(buffer);
    }
    while(*buffer != 0x00);
    free(buffer);
    return 0;
}

