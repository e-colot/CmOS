#pragma once

#include "disk.h"
#include "memory.h"
#include "cpu.h"

typedef struct {
    Ram* memory;
    Reg* reg;
} Computer;

Computer* boot();
void shutdown(Computer* computer);

