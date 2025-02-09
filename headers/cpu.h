#pragma once
#include "memory.h"

typedef struct {
    unsigned char* FLAGS, *R1, *R2, *R3, *R4, *R5, *RL, *RH;
    unsigned short* R16, *RSI, *RDI, *RI, *RS;
} Reg;

Reg* createReg();
void deleteReg(Reg* reg);

void setRI(Reg* reg, Ram* memory, unsigned short index);

int runCode(Reg* reg, Ram* memory);
