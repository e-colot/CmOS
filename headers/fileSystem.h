#pragma once
#include "constants.h"
#include <stdio.h>

// type used for addressing
typedef union {
    unsigned char bytes[ADDRESSING_BYTES];
    unsigned int value;  // must be large enough to fit ADDRESSING_BYTES
} AddressType;

AddressType getAddress(unsigned char* src);
void setAddress(unsigned char* dest, AddressType src);
unsigned char checkAddress(unsigned char* src, size_t value);

void getPage(AddressType pos, unsigned char* buffer);
void setPage(AddressType pos, unsigned char* buffer);

// Bitmap operations
AddressType getFreePage();
void updateBitmap(AddressType page);

// FAT operations
size_t addToFAT(AddressType ID, AddressType page);
AddressType removeFromFAT(AddressType ID);
AddressType searchFAT(AddressType ID);

void reorganizeFAT();
size_t getFileSize(AddressType ID);
size_t addFile(const char* filePath, AddressType ID);
size_t loadFile(AddressType ID, unsigned char* mem, size_t len);
size_t removeFile(AddressType ID);

