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
#include <string.h>
#include <stdio.h>

/* pipeline registers */
Pipe_Reg_IFtoDE  Reg_IFtoDE;
Pipe_Reg_DEtoEX  Reg_DEtoEX;
Pipe_Reg_EXtoMEM Reg_EXtoMEM;
Pipe_Reg_MEMtoWB Reg_MEMtoWB;

void pipe_init() {
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00000000;

    Reg_IFtoDE.valid  = 0;
    Reg_DEtoEX.valid  = 0;
    Reg_EXtoMEM.valid = 0;
    Reg_MEMtoWB.valid = 0;
}

void pipe_cycle() {
    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode();
    pipe_stage_fetch();
}

/************************ WB **************************/
void pipe_stage_wb() {
    if (!Reg_MEMtoWB.valid) return;

    if (Reg_MEMtoWB.reg_write && Reg_MEMtoWB.rd != 0) {
        if (Reg_MEMtoWB.is_load)
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.mem_data;
        else
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.alu_result;
    }

    stat_inst_retire++;
}

/************************ MEM *************************/
void pipe_stage_mem() {
    if (!Reg_EXtoMEM.valid) {
        Reg_MEMtoWB.valid = 0;
        return;
    }

    uint32_t mem_data = 0;

    if (Reg_EXtoMEM.is_load) {
        mem_data = mem_read_32(Reg_EXtoMEM.alu_result);
    }
    else if (Reg_EXtoMEM.is_store) {
        mem_write_32(Reg_EXtoMEM.alu_result, Reg_EXtoMEM.store_val);
    }

    Reg_MEMtoWB.alu_result = Reg_EXtoMEM.alu_result;
    Reg_MEMtoWB.mem_data   = mem_data;
    Reg_MEMtoWB.rd         = Reg_EXtoMEM.rd;
    Reg_MEMtoWB.is_load    = Reg_EXtoMEM.is_load;
    Reg_MEMtoWB.reg_write  = Reg_EXtoMEM.reg_write;
    Reg_MEMtoWB.valid      = 1;
}

/*********************** EXECUTE ***********************/
void pipe_stage_execute() {
    if (!Reg_DEtoEX.valid) {
        Reg_EXtoMEM.valid = 0;
        return;
    }

    uint32_t alu = 0;
    int op = Reg_DEtoEX.opcode;

    switch (op) {
        case 0x33:  // add
            alu = Reg_DEtoEX.rv1 + Reg_DEtoEX.rv2;
            break;

        case 0x13:  // addi
            alu = Reg_DEtoEX.rv1 + Reg_DEtoEX.imm;
            break;

        case 0x03:  // lw
        case 0x23:  // sw
            alu = Reg_DEtoEX.rv1 + Reg_DEtoEX.imm;
            break;

        case 0x63:  // blt
            alu = (Reg_DEtoEX.rv1 < Reg_DEtoEX.rv2);
            break;

        case 0x17:  // auipc
            alu = (Reg_DEtoEX.pc_plus4 - 4) + Reg_DEtoEX.imm;
            break;

        case 0x6F:  // jal (return address = pc+4)
            alu = Reg_DEtoEX.pc_plus4;
            break;
    }

    Reg_EXtoMEM.alu_result = alu;
    Reg_EXtoMEM.store_val  = Reg_DEtoEX.rv2;
    Reg_EXtoMEM.rd         = Reg_DEtoEX.rd;
    Reg_EXtoMEM.is_load    = Reg_DEtoEX.is_load;
    Reg_EXtoMEM.is_store   = Reg_DEtoEX.is_store;
    Reg_EXtoMEM.is_jal     = Reg_DEtoEX.is_jal;
    Reg_EXtoMEM.reg_write  = Reg_DEtoEX.reg_write;
    Reg_EXtoMEM.valid      = 1;
}

/*********************** DECODE ************************/
void pipe_stage_decode() {
    if (!Reg_IFtoDE.valid) {
        Reg_DEtoEX.valid = 0;
        return;
    }

    uint32_t instr = Reg_IFtoDE.instr;
    int op = instr & 0x7F;

    int rd  = (instr >> 7)  & 0x1F;
    int f3  = (instr >> 12) & 0x7;
    int rs1 = (instr >> 15) & 0x1F;
    int rs2 = (instr >> 20) & 0x1F;

    int32_t imm = 0;
    int isL=0, isS=0, isB=0, isJ=0, isU=0, wr=0;

    switch (op) {
        case 0x33: wr=1; break; // add
        case 0x13: wr=1; imm=(int32_t)instr>>20; break; // addi
        case 0x03: isL=1; wr=1; imm=(int32_t)instr>>20; break; // lw
        case 0x23: // sw
            imm = ((instr>>25)<<5)|((instr>>7)&0x1F);
            if (imm & 0x800) imm |= 0xFFFFF000;
            isS=1;
            break;
        case 0x63: // blt
            imm = ((instr>>31)<<12)|(((instr>>7)&1)<<11)|
                  (((instr>>25)&0x3F)<<5)|(((instr>>8)&0xF)<<1);
            if (imm & 0x1000) imm |= 0xFFFFE000;
            isB=1;
            break;
        case 0x17: // auipc
            imm = (int32_t)(instr & 0xFFFFF000);
            isU=1; wr=1;
            break;
        case 0x6F: // jal
            isJ=1; wr=1;
            imm = ((instr>>31)<<20)|(((instr>>12)&0xFF)<<12)|
                  (((instr>>20)&1)<<11)|(((instr>>21)&0x3FF)<<1);
            if (imm & 0x100000) imm |= 0xFFE00000;
            break;
    }

    Reg_DEtoEX.instr     = instr;
    Reg_DEtoEX.pc_plus4  = Reg_IFtoDE.pc_plus4;
    Reg_DEtoEX.opcode    = op;

    Reg_DEtoEX.rd  = rd;
    Reg_DEtoEX.rs1 = rs1;
    Reg_DEtoEX.rs2 = rs2;

    Reg_DEtoEX.imm = imm;
    Reg_DEtoEX.rv1 = CURRENT_STATE.REGS[rs1];
    Reg_DEtoEX.rv2 = CURRENT_STATE.REGS[rs2];

    Reg_DEtoEX.is_load  = isL;
    Reg_DEtoEX.is_store = isS;
    Reg_DEtoEX.is_branch= isB;
    Reg_DEtoEX.is_jal   = isJ;
    Reg_DEtoEX.is_auipc = isU;
    Reg_DEtoEX.reg_write= wr;
    Reg_DEtoEX.valid    = 1;
}

/************************ FETCH *************************/
void pipe_stage_fetch() {
    uint32_t instr = mem_read_32(CURRENT_STATE.PC);

    Reg_IFtoDE.instr   = instr;
    Reg_IFtoDE.pc_plus4= CURRENT_STATE.PC + 4;
    Reg_IFtoDE.valid   = 1;

    CURRENT_STATE.PC += 4;

    /* HALT condition: memory returns 0 and pipeline empty */
    if (instr == 0 &&
        !Reg_IFtoDE.valid &&
        !Reg_DEtoEX.valid &&
        !Reg_EXtoMEM.valid &&
        !Reg_MEMtoWB.valid)
    {
        RUN_BIT = 0;
    }
}
