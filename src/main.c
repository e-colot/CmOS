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

    if(writeTest(15, 33) == 1) {
        printf("Error in write test\n");
        shutdown(computer);
        return 1;
    }
    else {
        printf("Write test succeeded\n");
    }

    printf("FAT:\n");
    printFAT();

    shutdown(computer);
    return 0;
}



