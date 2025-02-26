#pragma once

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
    char* registers;
} PCB; // stands for process control block

PCB* creatPCB(unsigned char PID);
void deletePCB(PCB* input);


