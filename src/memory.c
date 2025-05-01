#include "memory.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>

Ram* createMemory() {
    Ram* memory = malloc(sizeof(Ram));
    memory->mem = malloc(RAM_SIZE);
    return memory;
}

void deleteMemory(Ram* memory) {
    free(memory->mem);
    free(memory);
}

