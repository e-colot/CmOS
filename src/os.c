#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "os.h"
#include "fileSystem.h"
#include "constants.h"
#include "processManagement.h"

Computer* boot() {

    static int seedInitialized = 0;
    if (!seedInitialized) {
        size_t seed = time(NULL);
        printf("Seed: %zu\n\n", seed);
        srand(seed);
        seedInitialized = 1;
    }

    diskInit(DISK_SIZE);

    Computer* comp = malloc(sizeof(Computer));

    comp->memory = createMemory();
    comp->processes = calloc(MAX_PROCESSES, sizeof(PCB*));
    
    printf("Computer booted\n");
    if (DISK_SIZE < 1024) {
        printf("Disk size: %zu B\n", (size_t) DISK_SIZE);
    } else if (DISK_SIZE < 1024 * 1024) {
        printf("Disk size: %zu kB\n", (size_t) DISK_SIZE / 1024);
    } else {
        printf("Disk size: %zu MB\n", (size_t) DISK_SIZE / (1024 * 1024));
    }

    if (PAGE_SIZE < 1024) {
        printf("Page size: %zu B\n\n", (size_t) PAGE_SIZE);
    } else {
        printf("Page size: %zu kB\n\n", (size_t) PAGE_SIZE / 1024);
    }
    return comp;
}

void shutdown(Computer* comp) {
    deleteMemory(comp->memory);
    for (size_t i = 0; i < MAX_PROCESSES; i+=1){
        if (*(comp->processes + i)){
            // if the PCB is not null (already deleted)
            deletePCB(*(comp->processes + i));
        }
    }
}


