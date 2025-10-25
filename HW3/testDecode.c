#include <stdio.h>
#include <stdint.h>

// Declare globals from sim.c
uint32_t INSTRUCTION;
uint32_t opcode, rd, rs1, rs2, funct3, funct7;
int32_t imm;
uint32_t rs1_val, rs2_val;
int RUN_BIT = 1;

typedef struct {
    uint32_t PC;
    uint32_t REGS[32];
} CPU_State;

CPU_State CURRENT_STATE, NEXT_STATE;

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

int main() {
    // Example: addi x5, x6, 12 -> 0x00C30313
    INSTRUCTION = 0x00C30313;
    CURRENT_STATE.REGS[6] = 42;  // x6 = 42

    decode();

    printf("opcode = 0x%x\n", opcode);
    printf("rd     = %d\n", rd);
    printf("rs1    = %d (val=%d)\n", rs1, rs1_val);
    printf("rs2    = %d (val=%d)\n", rs2, rs2_val);
    printf("funct3 = 0x%x\n", funct3);
    printf("funct7 = 0x%x\n", funct7);
    printf("imm    = %d\n", imm);

    return 0;
}
