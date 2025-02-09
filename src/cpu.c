#include "cpu.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

Reg* createReg() {
    Reg* reg = malloc(sizeof(Reg));

    reg->FLAGS = malloc(16);
    reg->R1 = reg->FLAGS + 1;
    reg->R2 = reg->FLAGS + 2;
    reg->R3 = reg->FLAGS + 3;
    reg->R4 = reg->FLAGS + 4;
    reg->R5 = reg->FLAGS + 5;
    reg->RL = reg->FLAGS + 6;
    reg->RH = reg->FLAGS + 7;
    reg->R16 = (unsigned short*)(reg->FLAGS + 6);
    reg->RSI = (unsigned short*)(reg->FLAGS + 8);
    reg->RDI = (unsigned short*)(reg->FLAGS + 10);
    reg->RI = (unsigned short*)(reg->FLAGS + 12);
    reg->RS = (unsigned short*)(reg->FLAGS + 14);

    return reg;
}

void deleteReg(Reg* reg) {
    if (reg) {
        free(reg->FLAGS);
        free(reg);
    }
}

void setRI(Reg* reg, Ram* memory, unsigned short index) {
    *(reg->RI) = 2 + *((unsigned short*)(memory->mem + index));
}

int runCode(Reg* reg, Ram* memory) {
    unsigned short index = *(reg->RI);
    unsigned char opcode = *(memory->mem + index);

    //DEBUG
    //printf("Opcode: %02X\nArg1: %02X\nArg2: %02X\n\n", opcode, *(memory->mem + index + 1), *(memory->mem + index + 2));

    switch (opcode) {
    case 0x00: // AND r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) & *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x01: // AND r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) & *(memory->mem + index + 2);
        break;
    case 0x02: // OR r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) | *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x03: // OR r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) | *(memory->mem + index + 2);
        break;
    case 0x04: // NOT r1 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = ~*(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4));
        break;
    case 0x05: // SHL r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) << *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x06: // SHL r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) << *(memory->mem + index + 2);
        break;
    case 0x07: // SHR r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) >> *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x08: // SHR r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) >> *(memory->mem + index + 2);
        break;
    case 0x10: // ADD r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) + *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x11: // ADD r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) + *(memory->mem + index + 2);
        break;
    case 0x12: // SUB r1, r2 -> r3
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) - *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x13: // SUB r1, imm8 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) - *(memory->mem + index + 2);
        break;
    case 0x14: // SUB imm8, r1 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 2) & 0x0F)) = *(memory->mem + index + 1) - *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4));
        break;
    case 0x15: // MUL r1, r2 -> R16
        *(reg->R16) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) * *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F));
        break;
    case 0x16: // MUL r1, imm8 -> R16
        *(reg->R16) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) * *(memory->mem + index + 2);
        break;
    case 0x17: // IDIV r1 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->R16) / *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4));
        break;
    case 0x18: // IDIV imm8 -> r1
        *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) = *(reg->R16) / *(memory->mem + index + 2);
        break;
    case 0x19: // MOD r1 -> r2
        *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) = *(reg->R16) % *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4));
        break;
    case 0x1A: // MOD imm8 -> r1
        *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) = *(reg->R16) % *(memory->mem + index + 2);
        break;
        case 0x20: // MOV r1 -> r2
        {
            unsigned char srcIndex = (*(memory->mem + index + 1) & 0xF0) >> 4;
            unsigned char destIndex = *(memory->mem + index + 1) & 0x0F;
        
            if ((srcIndex+destIndex) >= 14) {
                // Both are 16-bit registers
                *(unsigned short*)(reg->FLAGS + destIndex) = *(unsigned short*)(reg->FLAGS + srcIndex);
            } else {
                // 8-bit registers
                *(reg->FLAGS + destIndex) = *(reg->FLAGS + srcIndex);
            }
        
            // SPECIAL CASE: MOV R16 RI
            if (destIndex == 0x0E && srcIndex == 0x0C) {
                *(reg->RI) -= 3;
                // to counteract the increment of 3 in the end 
            }
            break;
        }
    case 0x21: // MOV imm8 -> r1
        *(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F))) = *(memory->mem + index + 2);
        break;
    case 0x22: // MOV imm16 -> R16
        *(reg->R16) = *(unsigned short*)(memory->mem + index + 1);
        break;
    case 0x23: // LOAD -> r1
        *(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F))) = *(memory->mem + 2 + *(reg->RSI));
        break;
    case 0x24: // LOAD -> R16
        *(reg->R16) = *(unsigned short*)(memory->mem + 2 + *(reg->RSI));
        break;
    case 0x25: // STORE r1 ->
        *(memory->mem + 2 + *(reg->RDI)) = *(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F)));
        break;
    case 0x26: // STORE R16 ->
        *(unsigned short*)(memory->mem + 2 + *(reg->RDI)) = *(reg->R16);
        break;
    case 0x27: // REGDUMP
        // Assuming RDI points to a memory location to store registers
        for (int i = 0; i < 16; i++) {
            *(memory->mem + 2 + *(reg->RDI) + i) = *((char*)reg + i);
        }
        break;
    case 0x28: // REGFILL
        // Assuming RSI points to a memory location to load registers from
        for (int i = 0; i < 16; i++) {
            *((char*)reg + i) = *(memory->mem + 2 + *(reg->RSI) + i);
        }
        break;
    case 0x30: // CMP r1, r2
        if (*(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)) - *(reg->FLAGS + ((*(memory->mem + index + 1) & 0xF0) >> 4)) == 0) {
            *(reg->FLAGS) |= 0x02; // Set ZF
        } else {
            *(reg->FLAGS) &= ~0x02; // Clear ZF
        }
        break;
    case 0x31: // TEST r1, r2
        if (*(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F))) & (1 << *(reg->FLAGS + (*(memory->mem + index + 1) & 0x0F)))) {
            *(reg->FLAGS) |= 0x02; // Set ZF
        } else {
            *(reg->FLAGS) &= ~0x02; // Clear ZF
        }
        break;
    case 0x32: // TEST r1, imm8
        if (*(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F))) & (1 << *(memory->mem + index + 2))) {
            *(reg->FLAGS) |= 0x02; // Set ZF
        } else {
            *(reg->FLAGS) &= ~0x02; // Clear ZF
        }
        break;
    case 0x40: // SKIFZ
        if (*(reg->FLAGS) & 0x02) {
            *(reg->RI) += 3; // Skip next instruction
        }
        break;
    case 0x41: // SKIFNZ
        if (!(*(reg->FLAGS) & 0x02)) {
            *(reg->RI) += 3; // Skip next instruction
        }
        break;
    case 0xF0: // PRNT r1
        for (int i = 0; i < *(reg->FLAGS + ((*(memory->mem + index + 1) & 0x0F))); i++) {
            if (i > 0) {
                printf("-");
            }
            printf("%d", *(memory->mem + 2 + *(reg->RSI) + i));
        }
        printf("\n");
        break;
    case 0xFF: // HLT
        return 0; // Stop the CPU
    default:
        perror("Unknown opcode");
        return 0; 
    }
    *(reg->RI) += 3; // Move to the next instruction
    return 1;
}
