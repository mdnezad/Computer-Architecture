/*
  Himadri Saha & Md Nezad
  EECE 4821 - Computer Architecture
  HW 4

  HS: Writing this down cause I keep messing it up
  RISC-V instruction feild (32-bit):
  31          25 24     20 19     15 14 12 11     7  6         0
  +-------------+---------+---------+-------+--------+---------+
  |   funct7    |   rs2   |   rs1   | funct3|   rd   | opcode |
  +-------------+---------+---------+-------+--------+---------+
   7 bits       5 bits    5 bits    3 bits  5 bits    7 bits

  Bit shifting:
  int feild = (instr >> lowest_bit_in_feild) & mask_of_feild_length;

*/ 
#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* global pipeline state */
// CPU_State CURRENT_STATE;
Pipe_Reg_IFtoDE Reg_IFtoDE;
Pipe_Reg_DEtoEX Reg_DEtoEX;
Pipe_Reg_EXtoMEM Reg_EXtoMEM;
Pipe_Reg_MEMtoWB Reg_MEMtoWB;

void pipe_init()
{
  memset(&CURRENT_STATE, 0, sizeof(CPU_State));
  CURRENT_STATE.PC = 0x00000000;
}

void pipe_cycle()
{
  pipe_stage_wb();
  pipe_stage_mem();
  pipe_stage_execute();
  pipe_stage_decode();
  pipe_stage_fetch();
}

void pipe_stage_wb()
{
}

void pipe_stage_mem()
{
}

void pipe_stage_execute()
{
}

/* Decode instruction (from fetch), reads regesiters, and generates control signals for ALU */
void pipe_stage_decode()
{
  // Check if its valid??
  if (!Reg_IFtoDE.valid) {
    Reg_DEtoEX.valid = 0;
    return;
  }

  // Get instruction and opcode 
  uint32_t instr = Reg_IFtoDE.instr;
  int opcode = instr & 0x7F;

  // Get instruction feilds
  int rd = (instr >> 7) & 0x1F;
  int funct3 = (instr >> 12) & 0x7;
  int rs1 = (instr >> 15) & 0x1F;
  int rs2 = (instr >> 20) & 0x1F;
  int funct7 = instr >> 25;

  // Determine what kind of instruction it is
  int32_t imm = 0;
  int is_load=0, is_store=0, is_branch=0, is_jal=0, is_auipc=0, reg_write=0;
  int alu_op=0;
  switch(opcode) {
    case 0x33:  // R-Type add
      alu_op = 1;
      reg_write = 1;  
      break;

    case 0x13: // addi
      imm = (int32_t)(instr & 0xFFF00000) >> 20;
      alu_op = 2;
      reg_write = 1;  
      break;

    case 0x03: // lw
      imm = (int32_t)(instr & 0xFFF00000) >> 20;
      is_load = 1;
      reg_write = 1;
      break;

    case 0x23: // sw
      imm = (((instr >> 25) & 0x7F) << 5) | ((instr >> 7) & 0x1F);
      if (imm & 0x800) {
        imm |= 0xFFFFF000;
      }
      is_store = 1;
      break;

    case 0x63: // blt
      is_branch = 1;
      imm = ((instr >> 31) << 12) |
            (((instr >> 7) & 1) << 11) |
            (((instr >> 25) & 0x3F) << 5) |
            (((instr >> 8) & 0xF) << 1);
      if (imm & 0x1000) {
        imm |= 0xFFFFE000;
      }
      break;

    case 0x17: // auipc
      imm = (int32_t)(instr & 0xFFFFF000);
      is_auipc = 1;
      reg_write = 1;
      break;

    case 0x6F: // jal
      is_jal = 1;
      reg_write = 1;
      imm = ((instr >> 31) << 20) |
            (((instr >> 12) & 0xFF) << 12) |
            (((instr >> 20) & 1) << 11) |
            (((instr >> 21) & 0x3FF) << 1);
      if (imm & 0x100000) {
        imm |= 0xFFE00000;
      }
      break;

  }

  // Place in DEtoEX reg
  Reg_DEtoEX.instr = instr;
  Reg_DEtoEX.pc_plus4 = Reg_IFtoDE.pc_plus4;
  
  Reg_DEtoEX.opcode = opcode;
  Reg_DEtoEX.rd = rd;
  Reg_DEtoEX.rs1 = rs1;
  Reg_DEtoEX.rs2 = rs2;
  Reg_DEtoEX.imm = imm;

  Reg_DEtoEX.reg_val1 = CURRENT_STATE.REGS[rs1];
  Reg_DEtoEX.reg_val2 = CURRENT_STATE.REGS[rs2];

  Reg_DEtoEX.is_load = is_load;
  Reg_DEtoEX.is_store = is_store;
  Reg_DEtoEX.is_branch = is_branch;
  Reg_DEtoEX.is_jal = is_jal;
  Reg_DEtoEX.is_auipc = is_auipc;
  Reg_DEtoEX.reg_write = reg_write;

  Reg_DEtoEX.valid = 1;

}

/* Send next instruction (+4) to decode */
void pipe_stage_fetch()
{
  // Get current instruction
  uint32_t instr = mem_read_32(CURRENT_STATE.PC);
  
  // update Pipe_Reg_IFtoDE registers
  Pipe_Reg_IFtoDE.instr = instr;
  Pipe_Reg_IFtoDE.pc_plus4 = instr + 4;

  // Update current state with next instruction
  CURRENT_STATE.PC = Pipe_Reg_IFtoDE.pc_plus4;
}