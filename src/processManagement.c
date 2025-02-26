#include "processManagement.h"
#include "cpu.h"

PCB* creatPCB(unsigned char PID) {
    PCB* output = malloc(sizeof(PCB));
    output->PID = PID;
    output->registers = calloc(16, 1); // calloc to have registers at '0'
    output->state = New;
    return output;
}

void deletePCB(PCB* input) {
    free(input->registers);
    free(input);
}

int runPCB(PCB* input, Ram* memory) {
    if (!runCode(input->registers, memory)) {
        input->state = Terminated;
        return 0;
    }
    return 1;
}





