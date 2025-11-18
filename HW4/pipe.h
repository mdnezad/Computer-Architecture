#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include <stdbool.h>

#define RISCV_REGS 32

typedef struct CPU_State_Struct {
    uint32_t PC;
    int32_t REGS[RISCV_REGS];
    int FLAG_NV;
    int FLAG_DZ;
    int FLAG_OF;
    int FLAG_UF;
    int FLAG_NX;
} CPU_State;

typedef struct {
    uint32_t instr;
    uint32_t pc_plus4;
    int valid;
} Pipe_Reg_IFtoDE;

typedef struct {
    uint32_t instr;
    uint32_t pc_plus4;

    int opcode;
    int rd, rs1, rs2;
    int32_t imm;
    int32_t rv1, rv2;

    int is_load;
    int is_store;
    int is_branch;
    int is_jal;
    int is_auipc;
    int reg_write;

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
extern CPU_State CURRENT_STATE;

void pipe_init();
void pipe_cycle();

void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif
