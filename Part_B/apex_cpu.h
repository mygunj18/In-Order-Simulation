/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int isStalled;      /* Gunj: added for checking stall status */
    int has_insn;
    int resetting_buffer;
    int New_rs1;
    int New_rs2;

    
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */ 
    int valid_bit[REG_FILE_SIZE];  /* Gunj added Valid bit indicator(0 and 1) */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int pos_flag;                  /* Positive flag */
    int zero_flag;                 /* Gunj added {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int forwardedDataBuffer[REG_FILE_SIZE];
    int fdata[REG_FILE_SIZE]; //to track pc updating bit 

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;

/*Gunj added*/
    int simulate;  // for enabling simulate function*/
    int opCycles; //Opration cycles to run definite cycles*/
    int memLoc; //to show value at particular memory location*/
    int showMem; // to show value at particular memory location*/

} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename, const char *op, const int no_of_cycles); //added by gunj for extra feature
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
