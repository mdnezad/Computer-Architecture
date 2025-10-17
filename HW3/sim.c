/*
  Himadri Saha
  EECE 4821 - Computer Architecture
  HW 3
*/
/* Imports */
#include <stdio.h>
#include "shell.h"

/* Vars */
uint32_t INSTRUCTION; // currently fetched instruction
uint32_t RUN_BIT;     // Set to 0 when no instruction to process (by process_instruction())

void fetch()
{
  // Increment 32-bit instruction from PC memory by 4 for the next instruction
  INSTRUCTION = mem_read_32(CURRENT_STATE.PC);
  NEXT_STATE.PC = CURRENT_STATE.PC + 4;
} 

void decode()
{
  
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
