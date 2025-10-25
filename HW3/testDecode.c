#include <stdio.h>
#include <stdint.h>

// Declare globals from sim.c (matching your current shapes)
uint32_t INSTRUCTION;
uint32_t opcode, rd, rs1, rs2, funct3, funct7;
int32_t  imm;
uint32_t rs1_val, rs2_val;
int RUN_BIT = 1;

typedef struct {
    uint32_t PC;
    uint32_t REGS[32];
} CPU_State;

CPU_State CURRENT_STATE, NEXT_STATE;

// -------- helpers --------
static inline int32_t signext(uint32_t val, int bits) {
    uint32_t m = 1u << (bits - 1);
    uint32_t mask = (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    val &= mask;
    return (int32_t)((val ^ m) - m);
}

void decode()
{
  /* Extract common fields and registers */
  opcode = INSTRUCTION & 0x7F;             // [6:0]
  rd     = (INSTRUCTION >> 7)  & 0x1F;     // [11:7]
  funct3 = (INSTRUCTION >> 12) & 0x07;     // [14:12]
  rs1    = (INSTRUCTION >> 15) & 0x1F;     // [19:15]
  rs2    = (INSTRUCTION >> 20) & 0x1F;     // [24:20]
  funct7 = (INSTRUCTION >> 25) & 0x7F;     // [31:25]

  rs1_val = CURRENT_STATE.REGS[rs1];
  rs2_val = CURRENT_STATE.REGS[rs2];

  /* Decode immediates per type */
  switch(opcode) {
    // R-type: add, slt
    case 0x33:
      imm = 0;
      break;

    // I-type: addi, slli
    case 0x13:
      if (funct3 == 0x1) {
        // SLLI: shamt = imm[4:0] (funct7 must be 0x00 for RV32I)
        imm = (INSTRUCTION >> 20) & 0x1F;
      } else {
        // ADDI: 12-bit signed immediate
        imm = signext(INSTRUCTION >> 20, 12);
      }
      break;

    // S-type: sw
    case 0x23: {
      uint32_t i12 = ((INSTRUCTION >> 25) & 0x7F) << 5;  // [31:25] -> imm[11:5]
      uint32_t i5  = (INSTRUCTION >> 7)  & 0x1F;         // [11:7]  -> imm[4:0]
      imm = signext(i12 | i5, 12);
      break;
    }

    // SB-type: bne
    case 0x63: {
      // imm = [31][7][30:25][11:8] << 1, sign-extended (13 bits total)
      uint32_t b12   = (INSTRUCTION >> 31) & 0x1;     // imm[12]
      uint32_t b11   = (INSTRUCTION >> 7)  & 0x1;     // imm[11]
      uint32_t b10_5 = (INSTRUCTION >> 25) & 0x3F;    // imm[10:5]
      uint32_t b4_1  = (INSTRUCTION >> 8)  & 0x0F;    // imm[4:1]
      uint32_t raw   = (b12 << 12) | (b11 << 11) | (b10_5 << 5) | (b4_1 << 1);
      imm = signext(raw, 13);
      break;
    }

    // U-type: auipc
    case 0x17:
      // upper 20 bits << 12 (already aligned)
      imm = (int32_t)(INSTRUCTION & 0xFFFFF000u);
      break;

    // UJ-type: jal
    case 0x6F: {
      // imm = [31][19:12][20][30:21] << 1, sign-extended (21 bits total)
      uint32_t b20    = (INSTRUCTION >> 31) & 0x1;   // imm[20]
      uint32_t b19_12 = (INSTRUCTION >> 12) & 0xFF;  // imm[19:12]
      uint32_t b11    = (INSTRUCTION >> 20) & 0x1;   // imm[11]
      uint32_t b10_1  = (INSTRUCTION >> 21) & 0x3FF; // imm[10:1]
      uint32_t raw    = (b20 << 20) | (b19_12 << 12) | (b11 << 11) | (b10_1 << 1);
      imm = signext(raw, 21);
      break;
    }

    default:
      // Unsupported for this test harness
      RUN_BIT = 0;
      break;
  }
}

int main() {
    // Example: addi x6, x6, 12 -> 0x00C30313 (rd=6, rs1=6, imm=12, funct3=000)
    INSTRUCTION = 0x00C30313;
    CURRENT_STATE.REGS[6] = 42;  // x6 = 42

    decode();

    printf("opcode = 0x%x\n", opcode);
    printf("rd     = %u\n", rd);
    printf("rs1    = %u (val=%u)\n", rs1, rs1_val);
    printf("rs2    = %u (val=%u)\n", rs2, rs2_val);
    printf("funct3 = 0x%x\n", funct3);
    printf("funct7 = 0x%x\n", funct7);
    printf("imm    = %d (0x%x)\n", imm, (uint32_t)imm);

    return 0;
}
