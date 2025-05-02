#pragma once
#include "constants.h"

// type used for addressing
typedef union {
    unsigned char bytes[ADDRESSING_BYTES];
    unsigned int value;  // must be large enough to fit ADDRESSING_BYTES
} AddressType;

AddressType getAddress(unsigned char* src);
void setAddress(unsigned char* dest, AddressType src);
unsigned char checkAddress(unsigned char* src, size_t value);


size_t getFileSize(AddressType ID);
void addFile(const char* filePath, AddressType ID);
void loadFile(AddressType ID, unsigned char* mem, size_t len);
void removeFile(AddressType ID);

