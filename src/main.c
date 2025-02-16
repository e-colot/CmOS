#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"

// should disappear later
#include "fileSystem.h"

void writeEraseTest(Computer* comp) {
    addFile("../programs/bin/multiplication", 0x25);
    loadFile(0x25, comp->memory->mem, 256);

    printBitmap();
    printf("\n");
    printFAT();

    removeFile(0x25);
    printf("File removed\n");

    printBitmap();
    printf("\n");
    printFAT();
}

void multiPageFATTest() {
    // test is 6 bytes -> 1 page per file
    addFile("../programs/bin/test", 0x25);
    addFile("../programs/bin/test", 0x26);
    addFile("../programs/bin/test", 0x27);
    addFile("../programs/bin/test", 0x28);
    addFile("../programs/bin/test", 0x29);
    addFile("../programs/bin/test", 0x2A);
    addFile("../programs/bin/test", 0x2B);

    printBitmap();
    printf("\n");
    printFAT();
    printf("\n\n");

    addFile("../programs/bin/test", 0x2C);
    addFile("../programs/bin/test", 0x2D);
    addFile("../programs/bin/test", 0x2E);

    printBitmap();
    printf("\n");
    printFAT();
}

removeFATPageTest() {
    addFile("../programs/bin/test", 0x25);
    addFile("../programs/bin/test", 0x26);
    addFile("../programs/bin/test", 0x27);
    addFile("../programs/bin/test", 0x28);
    addFile("../programs/bin/test", 0x29);
    addFile("../programs/bin/test", 0x2A);
    addFile("../programs/bin/test", 0x2B);

    printFAT();
    printf("\n\n");

    removeFile(0x2A);

    printBitmap();
    printf("\n");
    printFAT();
}

int main() {    
    
    Computer* comp = boot();

    // writeEraseTest(comp);
    multiPageFATTest();

    shutdown(comp);
    return 0;

}



