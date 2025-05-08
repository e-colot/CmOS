#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "fileSystem.h"

typedef struct {
    char* fileName;
    size_t fileSize;
} FileInfo;

typedef struct {
    unsigned char fileIndex;
    AddressType ID;
} FileEntry;

void createFiles(size_t* sizes, size_t count, FileInfo* fileInfos);
size_t writeDataToFile(const char* fileName, const unsigned char* data, size_t dataSize);
unsigned char timeToFillDisk(const char* fileName, size_t itr, unsigned char createNewFiles);
unsigned char normalOperationMeasurement(const char* fileName, size_t itr, unsigned char createNewFiles);
