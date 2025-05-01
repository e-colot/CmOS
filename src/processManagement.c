#include "processManagement.h"
#include "cpu.h"

PCB* createPCB(unsigned char PID) {
    PCB* output = malloc(sizeof(PCB));
    output->PID = PID;
    output->registers = createReg();
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





