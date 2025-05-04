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

    printf("FAT before adding entry:\n");
    printFAT();

    AddressType ID;
    AddressType length;

    // Adding the first entry
    ID.value = 0x25;
    length.value = 15;
    CA_FATEntry entry = {ID, length, length};
    CA_addToFAT(entry);

    // Adding 10 more entries with different values
    for (int i = 0; i < 10; i++) {
        ID.value = 0x30 + i; // Assign unique IDs
        length.value = 25 + i; // Assign varying lengths
        CA_FATEntry newEntry = {ID, length, length};
        CA_addToFAT(newEntry);
    }

    printf("FAT after adding entry:\n");
    printFAT();

    shutdown(computer);
    return 0;
}



