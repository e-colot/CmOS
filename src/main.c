#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"
#include "tests.h"
#include "fileSystem.h"
#include "contiguousAllocation.h"

int main(int argc, char* argv[]) {    
    
    Computer* computer = boot();

    // Check if the program is run in test mode
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        runTests();
        shutdown(computer);
        return 0;
    }

    if(multipleWriteEraseTest(100) == 1) {
        printf("Error in multiple write/erase test\n");
        shutdown(computer);
        return 1;
    }
    else {
        printf("Multiple write/erase test succeeded\n");
    }

    printf("FAT:\n");
    printFAT();

    shutdown(computer);
    return 0;
}



