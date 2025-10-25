/*
  Himadri Saha & Md Nezad
  EECE 4821 - Computer Architecture
  HW 3
*/

#include <stdint.h>
#include "shell.h"

uint32_t INSTRUCTION;
uint32_t opcode, rd, rs1, rs2, funct3, funct7;
int32_t imm;
int32_t rs1_val, rs2_val;

/******** Sign-extension helper ********/
static inline int32_t signext(uint32_t val, int bits) {
    uint32_t mask = (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    uint32_t m = 1u << (bits - 1);
    val &= mask;
    return (int32_t)((val ^ m) - m);
}

/******** Write register helper ********/
static inline void write_reg(uint32_t r, int32_t value){
    if (r != 0) NEXT_STATE.REGS[r] = value; // x0 hard-wired to 0
}

/******** FETCH ********/
void fetch() {
    INSTRUCTION = mem_read_32(CURRENT_STATE.PC);
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; // default
}

/******** DECODE ********/
void decode() {
    opcode = INSTRUCTION & 0x7F;
    rd     = (INSTRUCTION >> 7) & 0x1F;
    funct3 = (INSTRUCTION >> 12) & 0x07;
    rs1    = (INSTRUCTION >> 15) & 0x1F;
    rs2    = (INSTRUCTION >> 20) & 0x1F;
    funct7 = (INSTRUCTION >> 25) & 0x7F;

    rs1_val = CURRENT_STATE.REGS[rs1];
    rs2_val = CURRENT_STATE.REGS[rs2];

    switch(opcode) {
        case 0x33: // R-type
            imm = 0;
            break;

        case 0x13: // I-type
            if (funct3 == 0x1) { // slli
                imm = (INSTRUCTION >> 20) & 0x1F;
            }
            else { // addi
                imm = signext(INSTRUCTION >> 20, 12);
            }
            break;

        case 0x23: { // S-type: sw
            uint32_t high = ((INSTRUCTION >> 25) & 0x7F) << 5;
            uint32_t low  = (INSTRUCTION >> 7) & 0x1F;
            imm = signext(high | low, 12);
            break;
        }

        case 0x63: { // B-type: bne
            uint32_t bit12   = (INSTRUCTION >> 31) & 1;
            uint32_t bit11   = (INSTRUCTION >> 7)  & 1;
            uint32_t bits10_5 = (INSTRUCTION >> 25) & 0x3F;
            uint32_t bits4_1  = (INSTRUCTION >> 8)  & 0x0F;
            uint32_t raw = (bit12 << 12) | (bit11 << 11) |
                           (bits10_5 << 5) | (bits4_1 << 1);
            imm = signext(raw, 13);
            break;
        }

        case 0x17: // U-type: auipc
            imm = (int32_t)(INSTRUCTION & 0xFFFFF000);
            break;

        case 0x6F: { // UJ-type: jal
            uint32_t bit20    = (INSTRUCTION >> 31) & 1;
            uint32_t bits19_12 = (INSTRUCTION >> 12) & 0xFF;
            uint32_t bit11    = (INSTRUCTION >> 20) & 1;
            uint32_t bits10_1  = (INSTRUCTION >> 21) & 0x3FF;
            uint32_t raw = (bit20 << 20) | (bits19_12 << 12) |
                           (bit11 << 11) | (bits10_1 << 1);
            imm = signext(raw, 21);
            break;
        }

        default:
            break;
    }
}

/******** EXECUTE ********/
void execute() {
    uint32_t pc = CURRENT_STATE.PC;

    switch(opcode) {
        case 0x33: // R-type
            if (funct3 == 0x0 && funct7 == 0x00) {
                write_reg(rd, rs1_val + rs2_val); // add
            }
            else if (funct3 == 0x2 && funct7 == 0x00) {
                write_reg(rd, rs1_val < rs2_val); // slt signed
            }
            break;

        case 0x13: // I-type
            if (funct3 == 0x0) { // addi
                write_reg(rd, rs1_val + imm);
            }
            else if (funct3 == 0x1 && funct7 == 0x00) { // slli
                write_reg(rd, rs1_val << (imm & 0x1F));
            }
            break;

        case 0x23: { // sw
            if (funct3 == 0x2) {
                uint32_t addr = rs1_val + imm;
                mem_write_32(addr, CURRENT_STATE.REGS[rs2]);
            }
            break;
        }

        case 0x63: { // bne
            if (funct3 == 0x1 && rs1_val != rs2_val)
                NEXT_STATE.PC = pc + imm;
            break;
        }

        case 0x17: // auipc
            write_reg(rd, pc + imm);
            break;

        case 0x6F: // jal
            write_reg(rd, pc + 4);
            NEXT_STATE.PC = pc + imm;
            break;
    }

    NEXT_STATE.REGS[0] = 0; // enforce x0 = 0
}

/******** Top-level ********/
void process_instruction() {

    fetch();
    if (INSTRUCTION == 0) {
        RUN_BIT = FALSE;
        return;
    }

    decode();
    execute();
}
