#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "os.h"
#include "fileSystem.h"
#include "constants.h"

Computer* boot() {
    diskInit(DISK_SIZE);

    Ram* memory = createMemory();
    Reg* registers = createReg();
    Computer* computer = malloc(sizeof(Computer));

    computer->memory = memory;
    computer->reg = registers;

    return computer;
}

void shutdown(Computer* computer) {
    deleteMemory(computer->memory);
    deleteReg(computer->reg);
    free(computer);
}
