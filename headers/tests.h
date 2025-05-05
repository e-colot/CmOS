#pragma once
#include <stddef.h>

unsigned char writeTest(size_t fileNbr, size_t fileSize);
unsigned char eraseTest(size_t fileNbr, size_t fileSize);
unsigned char multipleWriteEraseTest(size_t maxMinItr);
unsigned char fatFillingTest(size_t fileNbr, size_t fileSize);
unsigned char fatReorganizationTest(size_t fileNbr, size_t fileSize);
void runTests();
