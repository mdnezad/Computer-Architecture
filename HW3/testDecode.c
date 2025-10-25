/*
  Himadri Saha & Md Nezad
  EECE 4821 - Computer Architecture
  HW 3
*/

#include <stdio.h>
#include <stdint.h>

uint32_t INSTRUCTION;
uint32_t opcode, rd, rs1, rs2, funct3, funct7;
int32_t imm;

static inline int32_t signext(uint32_t val, int bits){
    uint32_t m = 1u << (bits - 1);
    uint32_t mask = (bits == 32)?0xFFFFFFFFu:((1u<<bits)-1u);
    val &= mask;
    return (int32_t)((val ^ m) - m);
}

void decode() {
    opcode = INSTRUCTION & 0x7F;
    rd     = (INSTRUCTION >> 7)  & 0x1F;
    funct3 = (INSTRUCTION >> 12) & 0x07;
    rs1    = (INSTRUCTION >> 15) & 0x1F;
    rs2    = (INSTRUCTION >> 20) & 0x1F;
    funct7 = (INSTRUCTION >> 25) & 0x7F;

    switch(opcode) {
        case 0x33: imm = 0; break;

        case 0x13:
            if (funct3 == 0x1)
                imm = (INSTRUCTION >> 20) & 0x1F; // slli shamt
            else
                imm = signext(INSTRUCTION >> 20, 12); // addi
            break;

        case 0x23: {
            uint32_t high = ((INSTRUCTION >> 25)&0x7F)<<5;
            uint32_t low  = (INSTRUCTION>>7)&0x1F;
            imm = signext(high|low,12);
            break;
        }

        case 0x63: {
            uint32_t bit12=(INSTRUCTION>>31)&1;
            uint32_t bit11=(INSTRUCTION>>7)&1;
            uint32_t bits10_5=(INSTRUCTION>>25)&0x3F;
            uint32_t bits4_1=(INSTRUCTION>>8)&0x0F;
            uint32_t raw = (bit12<<12)|(bit11<<11)|(bits10_5<<5)|(bits4_1<<1);
            imm = signext(raw,13);
            break;
        }

        case 0x17:
            imm = (int32_t)(INSTRUCTION & 0xFFFFF000);
            break;

        case 0x6F: {
            uint32_t bit20=(INSTRUCTION>>31)&1;
            uint32_t bits19_12=(INSTRUCTION>>12)&0xFF;
            uint32_t bit11=(INSTRUCTION>>20)&1;
            uint32_t bits10_1=(INSTRUCTION>>21)&0x3FF;
            uint32_t raw=(bit20<<20)|(bits19_12<<12)|(bit11<<11)|(bits10_1<<1);
            imm = signext(raw,21);
            break;
        }
    }
}

void printDecoded(const char* name){
    printf("%s: inst=0x%08X, opcode=0x%02X, rd=%u, rs1=%u, rs2=%u, funct3=0x%X, imm=%d\n",
           name, INSTRUCTION, opcode, rd, rs1, rs2, funct3, imm);
}

int main(){
    uint32_t tests[] = {
        0x10000917, // auipc
        0x00090913, // addi
        0x015ea023, // sw
        0x002e1e93, // slli
        0x015a0b33, // add
        0x013e2f33, // slt
        0xfe0f10e3, // bne (-32)
        0x0040006f  // jal
    };
    const char *names[] = {
        "auipc","addi","sw","slli","add","slt","bne","jal"
    };

    for(int i=0;i<8;i++){
        INSTRUCTION=tests[i];
        decode();
        printDecoded(names[i]);
    }
    return 0;
}
