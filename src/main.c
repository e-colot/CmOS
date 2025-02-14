#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "constants.h"
#include "disk.h"
#include "memory.h"
#include "cpu.h"
#include "os.h"



int main() {    
    
    diskInit(DISK_SIZE);

    addFile("../programs/bin/multiplication", 0x25);

    // Ram* memory = createMemory();

    // diskRead(500, memory->mem, 0, 256);

    // Reg* reg = createReg();
    // setRI(reg, memory, 0);


    // while (runCode(reg, memory)) {
    //     //DEBUG
    //     //printReg(reg);
    // }
    
    // printf("End of program\n");

    // diskWrite(0, memory->mem, 256);
    // // stores the memory back to the disk for debugging purposes
    
    return 0;

}



