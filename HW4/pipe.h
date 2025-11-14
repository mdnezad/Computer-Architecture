#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

#define RISCV_REGS 32

typedef struct CPU_State_Struct {
  uint32_t PC;		          /* program counter */
  int32_t REGS[RISCV_REGS]; /* register file. */
  int FLAG_NV;              /* invalid operation */
  int FLAG_DZ;              /* divide by zero */
  int FLAG_OF;              /* overflow */
  int FLAG_UF;              /* underflow */
  int FLAG_NX;              /* inexact */
} CPU_State;

typedef struct {
  uint32_t instr;
  uint32_t pc_plus4; 

} Pipe_Reg_IFtoDE;

typedef struct {
  uint32_t instr;
  uint32_t pc_plus4;
  int opcode, rd, rs1, rs2;
  int32_t imm;
  int32_t reg_val1, reg_val2;
  int is_load, is_store, is_branch, is_jal, is_auipc, reg_write;
  int valid;

} Pipe_Reg_DEtoEX;

typedef struct {
  uint32_t alu_result;
  int32_t store_val;
  int rd;
  int is_load, is_store, is_jal, reg_write;
  int valid;

} Pipe_Reg_EXtoMEM;

typedef struct {
  uint32_t alu_result;
  uint32_t mem_data;
  int rd;
  int is_load;
  int reg_write;
  int valid;

} Pipe_Reg_MEMtoWB;


extern int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif