#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"

// should disappear later
#include "fileSystem.h"

void writeTest(Ram* memory) {
    addFile("../programs/bin/multiplication", 0x25);
    loadFile(0x25, memory->mem, 256);

    printBitmap();
    printf("\n");
    printFAT();
}

void writeEraseTest(Ram* memory) {
    addFile("../programs/bin/multiplication", 0x25);
    loadFile(0x25, memory->mem, 256);

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
    // test is < 15 bytes -> 1 page per file
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

void removeFATPageTest() {
    addFile("../programs/bin/test", 0x25);
    addFile("../programs/bin/test", 0x26);
    addFile("../programs/bin/test", 0x27);
    addFile("../programs/bin/test", 0x28);
    addFile("../programs/bin/test", 0x29);
    addFile("../programs/bin/test", 0x2A);
    addFile("../programs/bin/test", 0x2B);
    // this will be in the second FAT page
    addFile("../programs/bin/test", 0x2C);

    printFAT();
    printf("\n\n");

    // removes the program on the last FAT page
    // removeFile(0x2C);
    // removes a program on the first FAT page
    removeFile(0x27);

    printBitmap();
    printf("\n");
    printFAT();
}

int main() {    
    
    Computer* computer = boot();

    // writeTest(computer->memory);
    // writeEraseTest(computer->memory);
    // multiPageFATTest();
    removeFATPageTest();

    shutdown(computer);
    return 0;

}



