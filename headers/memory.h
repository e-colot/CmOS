#pragma once
#include <stdlib.h>

typedef struct {
    char* mem;
} Ram;

Ram* createMemory();
void deleteMemory(Ram* memory);
