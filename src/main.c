#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"
#include "tests.h"

// should disappear later
#include "fileSystem.h"


void writeEraseTest(Ram* memory) {
    AddressType ID;
    ID.value = 0x25;
    addFile("../programs/bin/multiplication", ID);
    loadFile(ID, memory->mem, 256);

    printBitmap();
    printf("\n");
    printFAT();

    removeFile(ID);
    printf("File removed\n");

    printBitmap();
    printf("\n");
    printFAT();
}

void multiPageFATTest() {
    // test is < 15 bytes -> 1 page per file
    AddressType ID;
    ID.value = 0x25;
    addFile("../programs/bin/test", ID);
    ID.value = 0x26;
    addFile("../programs/bin/test", ID);
    ID.value = 0x27;
    addFile("../programs/bin/test", ID);
    ID.value = 0x28;
    addFile("../programs/bin/test", ID);
    ID.value = 0x29;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2A;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2B;
    addFile("../programs/bin/test", ID);

    printBitmap();
    printf("\n");
    printFAT();
    printf("\n\n");

    ID.value = 0x2C;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2D;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2E;
    addFile("../programs/bin/test", ID);

    printBitmap();
    printf("\n");
    printFAT();
}

void removeFATPageTest() {
    AddressType ID;
    ID.value = 0x25;
    addFile("../programs/bin/test", ID);
    ID.value = 0x26;
    addFile("../programs/bin/test", ID);
    ID.value = 0x27;
    addFile("../programs/bin/test", ID);
    ID.value = 0x28;
    addFile("../programs/bin/test", ID);
    ID.value = 0x29;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2A;
    addFile("../programs/bin/test", ID);
    ID.value = 0x2B;
    addFile("../programs/bin/test", ID);
    // this will be in the second FAT page
    ID.value = 0x2C;
    addFile("../programs/bin/test", ID);

    printFAT();
    printf("\n\n");

    // removes the program on the last FAT page
    // removeFile(0x2C);
    // removes a program on the first FAT page
    ID.value = 0x27;
    removeFile(ID);

    printBitmap();
    printf("\n");
    printFAT();
}

int main() {    
    
    Computer* computer = boot();
    printf("\n\n");

    unsigned char res = writeTest(27, 132);
    if (res) {
        printf("Error in writeTest\n");
    }
    else {
        printf("writeTest passed\n");
    }
    // writeEraseTest(computer->memory);
    // multiPageFATTest();
    // removeFATPageTest();
    printf("\n\n");

    printf("Bitmap:\n");
    printBitmap();
    printf("FAT:\n");
    printFAT();

    shutdown(computer);
    return 0;

}



