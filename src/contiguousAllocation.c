#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "fileSystem.h"
#include "contiguousAllocation.h"
#include "disk.h"

// --------------- FAT operations ---------------
AddressType CA_addToFAT(CA_FATEntry entry) {
    // adds the entry [ID, page, length] to the FAT
    // returns the page address of the entry
    
    // with contiguous allocation, the FAT is at address 0
    // it starts with 0 (stored in the first ADDRESSING_BYTES bytes)
    // followed by the number of pages in the FAT (stored in the second ADDRESSING_BYTES bytes)
    // and it terminates with the last entry index in the FAT (stored in the third ADDRESSING_BYTES bytes)

    // each entry is 3*ADDRESSING_BYTES bytes long
    // the first ADDRESSING_BYTES bytes are the ID
    // the second ADDRESSING_BYTES bytes are the page address of the 1st page
    // the third ADDRESSING_BYTES bytes are the length of the file in pages

    // first try to put it at the end of the FAT
    // if it doesn't fit, try finding a sufficiently large free space
    // if not found, defragment the disk and try again

    AddressType zero = {0};
    CA_FATEntry FATinfo = CA_getFATEntry(zero);
    CA_FATEntry lastEntry = CA_getFATEntry(FATinfo.length);

    // check available space
    if (FATinfo.length.value == 0) {
        // no entry in FAT

        // place at the end of the disk
        entry.page.value = DISK_SIZE / PAGE_SIZE - entry.length.value;
        // set the new entry page
        AddressType entryLocation;
        entryLocation.value = 1;
        CA_setFATEntry(entryLocation, entry);

        // update the FAT length
        FATinfo.length.value = 1;
        size_t FATSizeBytes = (FATinfo.length.value+1)*3*ADDRESSING_BYTES;
        FATinfo.page.value = (FATSizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
        // set the FAT page
        CA_setFATEntry(zero, FATinfo);
        return entry.page;
    }
    else {
        // because the files are put from end to start, checking if the file fits is easy
        size_t lastPage = lastEntry.page.value;

        // +1 as FATinfo takes space too
        size_t endOfFAT = (FATinfo.length.value+1)*3*ADDRESSING_BYTES; // number of bytes taken by FAT
        endOfFAT = (endOfFAT + PAGE_SIZE - 1) / PAGE_SIZE; // number of pages taken by FAT
        if (lastPage-endOfFAT >= entry.length.value) {
            // there is enough space to store the new entry
            // add the entry to the FAT
            AddressType entryLocation;
            // put it after the last entry
            entryLocation.value = FATinfo.length.value + 1;

            // set the new entry page
            entry.page.value = lastPage - entry.length.value;
            CA_setFATEntry(entryLocation, entry);

            // update the FAT length
            FATinfo.length.value = FATinfo.length.value + 1;
            size_t FATSizeBytes = (FATinfo.length.value+1)*3*ADDRESSING_BYTES; 
            FATinfo.page.value = (FATSizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
            // set the FAT page
            CA_setFATEntry(zero, FATinfo);
            return entry.page;
        }
        // if not, we need to find a free space in between existing entries
        printf("Not enough space to add the entry\n");
        //TODO
    }
    // only happens in case of error
    return zero;
}

CA_FATEntry CA_getFATEntry(AddressType location) {
    // get the FAT entry at the given location
    unsigned char* buffer = malloc(3*ADDRESSING_BYTES);
    diskRead(location.value*3*ADDRESSING_BYTES, buffer, 0, 3*ADDRESSING_BYTES);
    // load the entry
    CA_FATEntry entry;
    entry.ID = getAddress(buffer);
    entry.page = getAddress(buffer + ADDRESSING_BYTES);
    entry.length = getAddress(buffer + 2*ADDRESSING_BYTES);
    // free the buffer
    free(buffer);
    return entry;
}

void CA_setFATEntry(AddressType location, CA_FATEntry entry) {
    // set the FAT entry at the given location
    unsigned char* buffer = malloc(3*ADDRESSING_BYTES);
    setAddress(buffer, entry.ID);
    setAddress(buffer + ADDRESSING_BYTES, entry.page);
    setAddress(buffer + 2*ADDRESSING_BYTES, entry.length);
    diskWrite(location.value*3*ADDRESSING_BYTES, buffer, 3*ADDRESSING_BYTES);
    free(buffer);
}

CA_FATEntry CA_searchFAT(AddressType ID) {
    // returns the page at which the file is stored
    AddressType index = {0};
    CA_FATEntry FATinfo = CA_getFATEntry(index);
    for (size_t i = 1; i <= FATinfo.length.value; i++) {
        // for each entry in the FAT
        index.value = i;
        CA_FATEntry entry = CA_getFATEntry(index);
        if (entry.ID.value == ID.value) {
            // the i-th entry corresponds to the ID to be found
            return entry;
        }
    }
    // if not found, return 0
    FATinfo.length.value = 0;
    return FATinfo;
}

// --------------- file operations ---------------
size_t CA_addFile(const char* filePath, AddressType ID) {

    //TODO: check for ID collisions
    // needs CA_searchFAT()

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
    size_t fileSize = fileStat.st_size;

    // file setup
    lseek(file, 0, SEEK_SET);

    // number of pages needed
    size_t pagesNbr = (fileSize + PAGE_SIZE - 1)/PAGE_SIZE;

    CA_FATEntry entry;
    entry.ID = ID;
    entry.length.value = pagesNbr;

    AddressType pageIndex = CA_addToFAT(entry);
    if (pageIndex.value == 0) {
        // no free page available
        printf("No free page available\n");
        close(file);
        return 1;
    }

    unsigned char* buffer = malloc(PAGE_SIZE);
    size_t bytesRead = 0;
    size_t destination = pageIndex.value*PAGE_SIZE;

    for (size_t i = 0; i < pagesNbr; i++) {
        // read the file into the buffer
        bytesRead = read(file, buffer, PAGE_SIZE);
        if (bytesRead < PAGE_SIZE) {
            // fill the remaining buffer with 0xFF
            memset(buffer + bytesRead, 0xFF, PAGE_SIZE - bytesRead);
        }
        // write the buffer to the disk
        diskWrite(destination, buffer, PAGE_SIZE);
        // update the destination
        destination += PAGE_SIZE;
    }
    free(buffer);
    close(file);
    return 0;
}

size_t CA_loadFile(AddressType ID, unsigned char* mem, size_t len) {
    CA_FATEntry location = CA_searchFAT(ID);
    if (location.page.value == 0) {
        // file not found
        printf("File not found\n");
        return 1;
    }
    // check if the memory is large enough
    if (len < location.length.value*PAGE_SIZE) {
        printf("Memory allocated to loadFile too small\n");
        return 1;
    }
    // load the file
    unsigned char* buffer = malloc(PAGE_SIZE);
    for (size_t i = 0; i < location.length.value; i++) {
        // read the page into the buffer
        diskRead(location.page.value*PAGE_SIZE, buffer, 0, PAGE_SIZE);
        // copy the buffer to the memory
        memcpy(mem + i*PAGE_SIZE, buffer, PAGE_SIZE);
        // update the page
        location.page.value++;
    }
    free(buffer);
    return 0;
}
