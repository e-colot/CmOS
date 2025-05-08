#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"
#include "tests.h"
#include "fileSystem.h"
#include "contiguousAllocation.h"
#include "measurements.h"

unsigned char FILE_ALLOCATION = 0;

int main(int argc, char* argv[]) {    
    
    Computer* computer = boot();

    // Check if the program is run in test mode
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {

        FILE_ALLOCATION = 0; // Set to bitmap + FAT for testing
        printf("\n      Running tests for linked allocation system\n");
        runTest();

        diskInit(DISK_SIZE); // Reinitialize the disk for contiguous allocation tests

        FILE_ALLOCATION = 1; // Set to contiguous allocation for testing
        printf("\n      Running tests for contiguous allocation system\n");
        runTest();

        shutdown(computer);
        return 0;
    }


    char filePath[256];
    char filePathCA[256];
    snprintf(filePath, sizeof(filePath), "../measurements/data/normalOp%d", PAGE_SIZE);
    snprintf(filePathCA, sizeof(filePath), "../measurements/data/normalOpCA%d", PAGE_SIZE);

    FILE_ALLOCATION = 0; // Set to bitmap + FAT for normal operation measurement
    diskInit(DISK_SIZE); // Initialize the disk for normal operation measurement
    printf("\033[1;33m\n      Running normal operation measurement for linked allocation system\n\033[0m");
    normalOperationMeasurement(filePath, 50, 0);

    FILE_ALLOCATION = 1; // Set to contiguous allocation for normal operation measurement
    diskInit(DISK_SIZE); // Initialize the disk for normal operation measurement
    printf("\033[1;33m\n      Running normal operation measurement for contiguous allocation system\n\033[0m");
    normalOperationMeasurement(filePathCA, 50, 0);

    shutdown(computer);
    return 0;
}



