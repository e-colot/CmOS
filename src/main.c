#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "disk.h"
#include "devTools.h"



int main() {    
    
    diskInit(DISK_SIZE);

    char text[] = "Stored in disk";
    size_t textLen = sizeof(text)-1;
    diskWrite(10, text, textLen);
    diskWrite(50, text, textLen);

    char ram[100];
    for (char i=0; i<100; i++) {
        ram[i] = (char)1;
    }
    printCharList(ram, (size_t)100);
    printf("\n\n");
    diskRead(5, ram, 10, 20);
    printCharList(ram, 100);

    return 0;

}



