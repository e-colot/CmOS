#pragma once

#include "disk.h"
#include "memory.h"
#include "cpu.h"
#include "processManagement.h"

typedef struct {
    Ram* memory;
    PCB** processes;
} Computer;

// ON/OFF procedures
Computer* boot();
void shutdown(Computer* memory);

// running porgrams
void addProcess(unsigned char PID);
