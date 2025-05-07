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

    // to avoid more than 1 recursive call
    static size_t recursiveCount = 0;

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
        // check if there is enough space at the end of the FAT

        // because the files are put from end to start, checking if the file fits is easy
        size_t lastPage = lastEntry.page.value;

        // +2 as FATinfo takes 1 entry and the new entry will also take 1 entry
        size_t endOfFAT = (FATinfo.length.value+2)*3*ADDRESSING_BYTES; // number of bytes taken by FAT
        endOfFAT = (endOfFAT + PAGE_SIZE - 1) / PAGE_SIZE; // number of pages taken by FAT
        if (lastPage >= endOfFAT && lastPage-endOfFAT >= entry.length.value) {
            // first part of the condition avoids negative values for lastPage-endOfFAT
            // there is enough space to store the new entry
            // add the entry to the FAT
            AddressType entryLocation;
            // put it after the last entry
            entryLocation.value = FATinfo.length.value + 1;

            // set the new entry page
            entry.page.value = lastPage - entry.length.value;
            CA_setFATEntry(entryLocation, entry);

            // update the FAT length
            FATinfo.length.value++;
            size_t FATSizeBytes = (FATinfo.length.value+1)*3*ADDRESSING_BYTES; 
            FATinfo.page.value = (FATSizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
            // set the FAT page
            CA_setFATEntry(zero, FATinfo);
            return entry.page;
        }
        // case of "file insertion"
        // check between existing entries to fit the new entry

        // it can only work if no new FAT page which would 'overflow' on the last file is created
        AddressType futureFATentries = FATinfo.length;
        futureFATentries.value++;
        size_t futureFATpageNbr = (futureFATentries.value+1)*3*ADDRESSING_BYTES;
        futureFATpageNbr = (futureFATpageNbr + PAGE_SIZE - 1) / PAGE_SIZE;
        size_t firstFilePage = CA_getFATEntry(FATinfo.length).page.value;
        if (futureFATpageNbr > firstFilePage) {
            // FAT page would overflow on the last file
            // defragmentation needed

            // need to recursively add the same file because after defragmentation, some disk space
            // might be freed at the beginning of the disk
            AddressType newTry = {0};
            if (recursiveCount == 0) {
                recursiveCount = 1;
                diskDefragmentation();
                newTry = CA_addToFAT(entry);
                recursiveCount = 0;
            }
            return newTry;
        }

        CA_FATEntry entryToCheck, nextEntry;
        AddressType entryToCheckIndex;
        // the first entry is at index 1
        // however, to be able to add a file even before the first entry the following must be done:
        // set the first entry index to 0
        entryToCheckIndex.value = 0;
        // set the first entry page to the maximal page on the disk
        entryToCheck.page.value = DISK_SIZE / PAGE_SIZE;
        while(entryToCheckIndex.value+1 <= FATinfo.length.value) {
            entryToCheckIndex.value++;
            nextEntry = CA_getFATEntry(entryToCheckIndex);
            // check if there is enough space between the two entries
            size_t space = entryToCheck.page.value - (nextEntry.page.value + nextEntry.length.value);
            if (space >= entry.length.value) {
                // there is enough space to store the new entry
                // add the entry to the FAT at entryToCheckIndex to maintain the order
                // this means all other entries need to be shifted

                // loads the new entry
                entry.page.value = nextEntry.page.value + nextEntry.length.value;
                CA_FATEntry entryToShift = entry;
                while (entryToCheckIndex.value <= FATinfo.length.value) {
                    // load previous entry
                    CA_FATEntry tmp = CA_getFATEntry(entryToCheckIndex);
                    // set the new entry
                    CA_setFATEntry(entryToCheckIndex, entryToShift);
                    // update the entry to shift
                    entryToShift = tmp;
                    // update the entry index
                    entryToCheckIndex.value++;
                }
                // still need to set the last entry
                CA_setFATEntry(entryToCheckIndex, entryToShift);
                // update the FAT length
                FATinfo.length.value++;
                size_t FATSizeBytes = (FATinfo.length.value+1)*3*ADDRESSING_BYTES;
                FATinfo.page.value = (FATSizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
                // set the FAT page
                CA_setFATEntry(zero, FATinfo);
                return entry.page;
            }
            // load the entru for the next iteration
            entryToCheck = CA_getFATEntry(entryToCheckIndex);
        }
        // if not, we need to defragment the disk
        diskDefragmentation();
    }
    // only happens in case of error
    return zero;
}

void CA_removeFromFAT(AddressType index) {
    // remove the entry located at index from the FAT

    // loads the FAT info
    AddressType zero = {0};
    CA_FATEntry FATinfo = CA_getFATEntry(zero);

    // shifts everything after it to the left
    index.value++;
    CA_FATEntry entryToShift = CA_getFATEntry(index);
    while (index.value <= FATinfo.length.value) {
        // set the new entry
        index.value--;
        CA_setFATEntry(index, entryToShift);
        // load the next entry
        index.value += 2;
        entryToShift = CA_getFATEntry(index);
    }
    // reduce the FAT length by 1
    FATinfo.length.value--;
    size_t FATSizeBytes = (FATinfo.length.value+1)*3*ADDRESSING_BYTES;
    FATinfo.page.value = (FATSizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
    // set the FAT page
    CA_setFATEntry(zero, FATinfo);
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
    // returns the FAT entry corresponding to the given ID
    // as the ID is known by the caller, it is replaced by the index of the entry
    // if not found, returns an entry with length = 0
    AddressType index = {0};
    CA_FATEntry FATinfo = CA_getFATEntry(index);
    for (size_t i = 1; i <= FATinfo.length.value; i++) {
        // for each entry in the FAT
        index.value = i;
        CA_FATEntry entry = CA_getFATEntry(index);
        if (entry.ID.value == ID.value) {
            // the i-th entry corresponds to the ID to be found
            // copy the page address to be returned
            entry.ID = index;
            return entry;
        }
    }
    // if not found, return 0
    FATinfo.page.value = 0;
    return FATinfo;
}




size_t diskDefragmentation() {
    // cycles through each entry and put it back as close as possible to the last one
    AddressType zero = {0};
    CA_FATEntry FATinfo = CA_getFATEntry(zero);

    AddressType lastAddress;
    lastAddress.value = DISK_SIZE/PAGE_SIZE;

    for (size_t i = 1; i <= FATinfo.length.value; i++) {
        AddressType index;
        index.value = i;
        CA_FATEntry currentEntry = CA_getFATEntry(index);
        // check if there is indeed space
        if (currentEntry.page.value + currentEntry.length.value == lastAddress.value) {
            // already spacd as it should, nothing must be done
            lastAddress = currentEntry.page;
            continue;
        }
        else if (currentEntry.page.value + currentEntry.length.value > lastAddress.value) {
            // file must go back, should NEVER happen
            printf("Corrupted FAT entries\n");
            return 1;
        }
        else {
            // the file must be moved

            // load the file
            unsigned char* buffer = malloc(PAGE_SIZE * currentEntry.length.value);
            // read the file into the buffer
            diskRead(currentEntry.page.value * PAGE_SIZE, buffer, 0, PAGE_SIZE * currentEntry.length.value);

            // change its address in FAT
            currentEntry.page.value = lastAddress.value - currentEntry.length.value;
            CA_setFATEntry(index, currentEntry);

            // add the file to disk
            size_t destination = currentEntry.page.value * PAGE_SIZE;
            for (size_t i = 0; i < currentEntry.length.value; i++) {
                // write the buffer to the disk
                diskWrite(destination, buffer+i*PAGE_SIZE, PAGE_SIZE);
                // update the destination
                destination += PAGE_SIZE;
            }
            free(buffer);

            lastAddress = currentEntry.page;
        }
    }
    return 0;
}



// --------------- file operations ---------------
size_t CA_addFile(const char* filePath, AddressType ID) {

    // check for ID collision
    CA_FATEntry entryWithSameID = CA_searchFAT(ID);
    if (entryWithSameID.page.value != 0) {
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
    // + 1 to account for the terminator
    size_t fileSize = fileStat.st_size + 1;

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
            // add a terminator
            buffer[bytesRead] = 0x00;
            // fill the remaining buffer with 0xFF
            memset(buffer + bytesRead + 1, 0xFF, PAGE_SIZE - bytesRead - 1);
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
    if (len < (location.length.value-1)*PAGE_SIZE) {
        printf("Memory allocated to loadFile too small\n");
        return 1;
    }
    // load the file
    unsigned char* buffer = malloc(PAGE_SIZE);
    for (size_t i = 0; i < location.length.value-1; i++) {
        // read the page into the buffer
        diskRead(location.page.value*PAGE_SIZE, buffer, 0, PAGE_SIZE);
        // copy the buffer to the memory
        memcpy(mem + i*PAGE_SIZE, buffer, PAGE_SIZE);
        // update the page
        location.page.value++;
    }
    // last page
    diskRead(location.page.value*PAGE_SIZE, buffer, 0, PAGE_SIZE);
    unsigned char terminatorSeen = 0;
    for (size_t i = PAGE_SIZE-1; i != 0xFFFFFFFF; i--) {
        if (buffer[i] == 0xFF) {
            // filling bytes
            continue;
        }
        if (terminatorSeen == 0 && buffer[i] == 0x00) {
            // terminator found
            terminatorSeen = 1;
            // check for destination buffer length
            if ((location.length.value-1)*PAGE_SIZE + i > len) {
                printf("Memory allocated to loadFile too small\n");
                free(buffer);
                return 1;
            }
            continue;
        }
        // copy the buffer to the memory (up to the terminator)
        memcpy(mem + (location.length.value-1)*PAGE_SIZE, buffer, i+1);
        break;
    }
    free(buffer);
    return 0;
}

size_t CA_removeFile(AddressType ID) {
    CA_FATEntry entry = CA_searchFAT(ID);
    if (entry.page.value == 0) {
        // file not found
        printf("Trying to remove a non-existing file\n");
        return 1;
    }
    CA_removeFromFAT(entry.ID);
    return 0;
}
