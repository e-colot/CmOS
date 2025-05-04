#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devTools.h"
#include "os.h"
#include "tests.h"


int main() {    
    
    Computer* computer = boot();

    runTests();

    shutdown(computer);
    return 0;

}



