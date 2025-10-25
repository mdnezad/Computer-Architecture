/*
  Himadri Saha & Md Nezad
  EECE 4821 - Computer Architecture
  HW 3
*/

/*
  RISC-V RV32I Single-Cycle Simulator (subset)
  Subset implemented: add, slt, addi, slli, sw, bne, auipc, jal

 */

#include <stdio.h>
#include <stdint.h>
#include "shell.h"

/* ===== Globals from your current file structure ===== */
uint32_t INSTRUCTION;                           // currently fetched instruction
uint32_t RUN_BIT;                               // set to 0 to halt
uint32_t opcode, rd, rs1, rs2, funct3, funct7;  // decode fields
int32_t  rs1_val, rs2_val;                      // source register values
int32_t  imm;                                   // decoded immediate

/* ===== Helpers ===== */
static inline int32_t signext(uint32_t val, int bits) {
  uint32_t m = 1u << (bits - 1);
  uint32_t mask = (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
  val &= mask;
  return (int32_t)((val ^ m) - m);
}

static inline void write_reg(uint32_t r, int32_t value) {
  if (r != 0) NEXT_STATE.REGS[r] = value; // x0 is hard-wired to 0
}

/* ===== Pipeline stages ===== */
void fetch() {
  INSTRUCTION = mem_read_32(CURRENT_STATE.PC);

  if (INSTRUCTION == 0u) {
    // No instruction to process -> halt (per assignment)
    RUN_BIT = FALSE;
    return;
  }

  // Default next PC (may be overridden by branch/jump)
  NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode() {
  if (RUN_BIT == FALSE) return;

  /* Extract common fields */
  opcode = INSTRUCTION & 0x7F;            // [6:0]
  rd     = (INSTRUCTION >> 7)  & 0x1F;    // [11:7]
  funct3 = (INSTRUCTION >> 12) & 0x07;    // [14:12]
  rs1    = (INSTRUCTION >> 15) & 0x1F;    // [19:15]
  rs2    = (INSTRUCTION >> 20) & 0x1F;    // [24:20]
  funct7 = (INSTRUCTION >> 25) & 0x7F;    // [31:25]

  rs1_val = CURRENT_STATE.REGS[rs1];
  rs2_val = CURRENT_STATE.REGS[rs2];
  imm     = 0;

  /* Decode immediates by type */
  switch (opcode) {
    case 0x33: /* R-type (add, slt) */
      imm = 0;
      break;

    case 0x13: /* I-type (addi, slli) */
      if (funct3 == 0x1) {
        // SLLI: shamt is imm[4:0]; funct7 must be 0x00 in RV32I
        imm = (INSTRUCTION >> 20) & 0x1F;
      } else {
        // ADDI: sign-extend 12 bits
        imm = signext(INSTRUCTION >> 20, 12);
      }
      break;

    case 0x23: { /* S-type (sw) */
      uint32_t i12 = ((INSTRUCTION >> 25) & 0x7F) << 5;
      uint32_t i5  = (INSTRUCTION >> 7) & 0x1F;
      imm = signext(i12 | i5, 12);
      break;
    }

    case 0x63: { /* SB-type (bne) */
      uint32_t b12    = (INSTRUCTION >> 31) & 0x1;      // imm[12]
      uint32_t b11    = (INSTRUCTION >> 7)  & 0x1;      // imm[11]
      uint32_t b10_5  = (INSTRUCTION >> 25) & 0x3F;     // imm[10:5]
      uint32_t b4_1   = (INSTRUCTION >> 8)  & 0x0F;     // imm[4:1]
      uint32_t raw    = (b12 << 12) | (b11 << 11) | (b10_5 << 5) | (b4_1 << 1);
      imm = signext(raw, 13);
      break;
    }

    case 0x17: /* U-type (auipc) */
      // upper 20 bits << 12
      imm = (int32_t)(INSTRUCTION & 0xFFFFF000u);
      break;

    case 0x6F: { /* UJ-type (jal) */
      uint32_t b20    = (INSTRUCTION >> 31) & 0x1;      // imm[20]
      uint32_t b19_12 = (INSTRUCTION >> 12) & 0xFF;     // imm[19:12]
      uint32_t b11    = (INSTRUCTION >> 20) & 0x1;      // imm[11]
      uint32_t b10_1  = (INSTRUCTION >> 21) & 0x3FF;    // imm[10:1]
      uint32_t raw    = (b20 << 20) | (b19_12 << 12) | (b11 << 11) | (b10_1 << 1);
      imm = signext(raw, 21);
      break;
    }

    default:
      // Not in the tested subset; per assignment inputs will be valid.
      // To be safe, halt.
      RUN_BIT = FALSE;
      break;
  }
}

void execute() {
  if (RUN_BIT == FALSE) return;

  uint32_t pc = CURRENT_STATE.PC;

  switch (opcode) {
    case 0x33: { /* R-type: add, slt */
      if (funct3 == 0x0 && funct7 == 0x00) {          /* ADD */
        write_reg(rd, (int32_t)((uint32_t)rs1_val + (uint32_t)rs2_val));
      } else if (funct3 == 0x2 && funct7 == 0x00) {   /* SLT (signed) */
        write_reg(rd, (rs1_val < rs2_val) ? 1 : 0);
      }
      break;
    }

    case 0x13: { /* I-type: addi, slli */
      if (funct3 == 0x0) {                            /* ADDI */
        write_reg(rd, rs1_val + imm);
      } else if (funct3 == 0x1 && funct7 == 0x00) {   /* SLLI (RV32I) */
        uint32_t shamt = (uint32_t)imm & 0x1F;
        write_reg(rd, (int32_t)((uint32_t)rs1_val << shamt));
      }
      break;
    }

    case 0x23: { /* S-type: sw */
      if (funct3 == 0x2) {                            /* SW */
        uint32_t addr = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs1] + imm);
        mem_write_32(addr, (uint32_t)CURRENT_STATE.REGS[rs2]);
      }
      break;
    }

    case 0x63: { /* SB-type: bne */
      if (funct3 == 0x1) {                            /* BNE */
        if (CURRENT_STATE.REGS[rs1] != CURRENT_STATE.REGS[rs2]) {
          NEXT_STATE.PC = pc + (uint32_t)imm;
        }
      }
      break;
    }

    case 0x17: { /* U-type: auipc */
      write_reg(rd, (int32_t)((uint32_t)pc + (uint32_t)imm));
      break;
    }

    case 0x6F: { /* UJ-type: jal */
      write_reg(rd, (int32_t)(pc + 4));
      NEXT_STATE.PC = pc + (uint32_t)imm;
      break;
    }

    default:
      // Shouldn’t reach here with valid inputs
      RUN_BIT = FALSE;
      break;
  }

  // Enforce x0 = 0
  NEXT_STATE.REGS[0] = 0;
}

void process_instruction() {
  /* Execute one instruction per call.
     Use CURRENT_STATE for reads, write results into NEXT_STATE. */
  fetch();
  if (RUN_BIT == FALSE) return;   // nothing to do

  decode();
  if (RUN_BIT == FALSE) return;

  execute();
}
