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
int32_t imm_i, imm_s, imm_b, imm_u, imm_j;      // immediate tags

void fetch()
{
  // Increment 32-bit instruction from PC memory by 4 for the next instruction
  INSTRUCTION = mem_read_32(CURRENT_STATE.PC);
  NEXT_STATE.PC = CURRENT_STATE.PC + 4;
} 

void decode()
{
  // Break down instruction into feilds and read register values 
  opcode = INSTRUCTION & 0x7F;             // bits [6:0]
  rd     = (INSTRUCTION >> 7) & 0x1F;      // [11:7]
  funct3 = (INSTRUCTION >> 12) & 0x7;      // [14:12]
  rs1    = (INSTRUCTION >> 15) & 0x1F;     // [19:15]
  rs2    = (INSTRUCTION >> 20) & 0x1F;     // [24:20]
  funct7 = (INSTRUCTION >> 25) & 0x7F;     // [31:25]
  rs1_val = CURRENT_STATE.REGS[rs1];
  rs2_val = CURRENT_STATE.REGS[rs2];

  /* Decode for immediate instructions */
  // I-Type (addi, slli, ...)
  imm_i = (int32_t)INSTRUCTION >> 20;

  // S-Type (sw)
  imm_s =  (INSTRUCTION >> 7) & 0x1F | (((int32_t)INSTRUCTION >> 25) << 5);

  


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
