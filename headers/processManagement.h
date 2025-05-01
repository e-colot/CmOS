#pragma once

#include "cpu.h"

enum PCBState {
    New,
    Ready,
    Running,
    Waiting,
    Terminated
};

typedef struct {
    enum PCBState state;
    unsigned char PID;   // process ID
    Reg* registers;
} PCB; // stands for process control block

PCB* createPCB(unsigned char PID);
void deletePCB(PCB* input);


