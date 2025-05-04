#include <stdio.h>
#include <stdlib.h>

#include "fileSystem.h"
#include "contiguousAllocation.h"
#include "disk.h"


void CA_addToFAT(CA_FATEntry entry) {
    // adds the entry [ID, page, length] to the FAT
    
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

    //TODO: check for ID collisions

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
        return;
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
            return;
        }
        // if not, we need to find a free space in between existing entries
        printf("Not enough space to add the entry\n");
        //TODO
    }
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

CA_FATEntry CA_setFATEntry(AddressType location, CA_FATEntry entry) {
    // set the FAT entry at the given location
    unsigned char* buffer = malloc(3*ADDRESSING_BYTES);
    setAddress(buffer, entry.ID);
    setAddress(buffer + ADDRESSING_BYTES, entry.page);
    setAddress(buffer + 2*ADDRESSING_BYTES, entry.length);
    diskWrite(location.value*3*ADDRESSING_BYTES, buffer, 3*ADDRESSING_BYTES);
    free(buffer);
    return entry;
}
