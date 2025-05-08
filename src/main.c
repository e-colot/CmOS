#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"
#include "tests.h"
#include "fileSystem.h"
#include "contiguousAllocation.h"
#include "measurements.h"


int main(int argc, char* argv[]) {    
    
    Computer* computer = boot();

    // Check if the program is run in test mode
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        runTest();
        shutdown(computer);
        return 0;
    }


    char filePath[256];
    if (FILE_ALLOCATION == 0) {
        snprintf(filePath, sizeof(filePath), "../measurements/data/fill%d", PAGE_SIZE);
    } else {
        snprintf(filePath, sizeof(filePath), "../measurements/data/fillCA%d", PAGE_SIZE);
    }
    timeToFillDisk("toDelete", 1, 0);

    shutdown(computer);
    return 0;
}



