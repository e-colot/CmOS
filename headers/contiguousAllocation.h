# pragma once

#include "fileSystem.h"
#include <stdio.h>

// every function will start with CA_ to avoid name collisions
// (CA_ = Contiguous Allocation)

typedef struct {
    AddressType ID;
    AddressType page;
    AddressType length;
} CA_FATEntry;

// FAT operations
AddressType CA_addToFAT(CA_FATEntry entry);
CA_FATEntry CA_getFATEntry(AddressType location);
void CA_setFATEntry(AddressType location, CA_FATEntry entry);
CA_FATEntry CA_searchFAT(AddressType ID);

// File operations
size_t CA_addFile(const char* filePath, AddressType ID);
size_t CA_loadFile(AddressType ID, unsigned char* mem, size_t len);
