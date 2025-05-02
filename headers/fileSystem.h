#pragma once
#include "constants.h"

// type used for addressing
typedef union {
    unsigned char bytes[ADDRESSING_BYTES];
    unsigned int value;  // must be large enough to fit ADDRESSING_BYTES
} AddressType;

void copyToAddress(unsigned char* src, AddressType* dest);
void copyFromAddress(AddressType* src, unsigned char* dest);
unsigned char checkAddress(void* src, size_t value);


size_t getFileSize(AddressType ID);
void addFile(const char* filePath, AddressType ID);
void loadFile(AddressType ID, unsigned char* mem, size_t len);
void removeFile(AddressType ID);

