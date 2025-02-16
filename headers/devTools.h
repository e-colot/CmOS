#pragma once
#include <stdio.h>
#include "cpu.h" // For Reg


void printCharList(unsigned char* list, size_t len);
void printReg(Reg* reg);
void printMem(Ram* memory);
void printBitmap();
void printFAT();
