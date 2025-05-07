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

    timeToFillDisk("../measurements/data/fill512", 50, 0);

    shutdown(computer);
    return 0;
}



