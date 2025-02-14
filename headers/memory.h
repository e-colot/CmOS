#pragma once
#include <stdlib.h>

typedef struct {
    unsigned char* mem;
} Ram;

Ram* createMemory();
void deleteMemory(Ram* memory);
