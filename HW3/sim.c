/*
  Himadri Saha & Md Nezad
  EECE 4821 - Computer Architecture
  HW 3
*/

/* Imports */
#include <stdio.h>
#include "shell.h"

/* Vars */
uint32_t INSTRUCTION;                           // currently fetched instruction
uint32_t RUN_BIT;                               // Set to 0 when no instruction to process (by process_instruction())
uint32_t opcode, rd, rs1, rs2, funct3, funct7;  // decode feilds  
int32_t rs1_val, rs2_val;                       // decode registers
int32_t imm;                                    // immediate feild

void fetch()
{
  // Increment 32-bit instruction from PC memory by 4 for the next instruction
  INSTRUCTION = mem_read_32(CURRENT_STATE.PC);
  NEXT_STATE.PC = CURRENT_STATE.PC + 4;
} 

void decode()
{
  /* Extract common feilds and registers */
  opcode = INSTRUCTION & 0x7F;             // bits [6:0]
  rd     = (INSTRUCTION >> 7) & 0x1F;      // [11:7]
  funct3 = (INSTRUCTION >> 12) & 0x7;      // [14:12]
  rs1    = (INSTRUCTION >> 15) & 0x1F;     // [19:15]
  rs2    = (INSTRUCTION >> 20) & 0x1F;     // [24:20]
  funct7 = (INSTRUCTION >> 25) & 0x7F;     // [31:25]
  rs1_val = CURRENT_STATE.REGS[rs1];
  rs2_val = CURRENT_STATE.REGS[rs2];

  /* Decode for immediate instructions */
  switch(opcode) {
    // R-type no immediate (add, slt)
    case 0x33: 
      imm = 0; 
      break;
    
    // I-type (addi, slli)
    case 0x13: 
      imm = (int32_t)INSTRUCTION >> 20;   // imm = bits [31:20]
      break;

    // S-type: sw
    case 0x23: 
      imm = (((int32_t)INSTRUCTION >> 25) << 5) | ((INSTRUCTION >> 7) & 0x1F); // imm = [31:25][11:7]
      break;

    // SB-type: bne
    case 0x63: 
      // imm = [31][7][30:25][11:8], sign extend from 13 bits
      imm = (((INSTRUCTION >> 31) & 0x1) << 12) |
            (((INSTRUCTION >> 7)  & 0x1) << 11) |
            (((INSTRUCTION >> 25) & 0x3F) << 5) |
            (((INSTRUCTION >> 8)  & 0xF) << 1);
      imm = (int32_t)(imm << 19) >> 19;
      break;

    case 0x17: // U-type: auipc
      imm = INSTRUCTION & 0xFFFFF000; // imm = bits [31:12] << 12
      break;

    case 0x6F: // UJ-type: jal
        // imm = [31][19:12][20][30:21], sign-extended
        imm = (((INSTRUCTION >> 31) & 0x1) << 20) |
              (((INSTRUCTION >> 12) & 0xFF) << 12) |
              (((INSTRUCTION >> 20) & 0x1) << 11) |
              (((INSTRUCTION >> 21) & 0x3FF) << 1);
        imm = (int32_t)(imm << 11) >> 11;  // sign extend from 21 bits
        break;

    default:
        // If an unsupported opcode shows up, stop simulator
        RUN_BIT = 0;
        break;

  } 

  /* Test decode output */

}

void execute()
{
  
}

void process_instruction()
{
  /* execute one instruction here. You should use CURRENT_STATE and modify
   * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
   * access memory. */
  fetch();
  decode();
  execute();
}
