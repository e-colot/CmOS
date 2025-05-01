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
    diskInit(DISK_SIZE);

    Computer* comp = malloc(sizeof(Computer));

    comp->memory = createMemory();
    comp->processes = calloc(MAX_PROCESSES, sizeof(PCB*));
    
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


