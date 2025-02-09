#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

Ram* createMemory() {
    Ram* memory = malloc(sizeof(Ram));
    memory->mem = malloc(256);
    return memory;
}

void deleteMemory(Ram* memory) {
    free(memory->mem);
    free(memory);
}

