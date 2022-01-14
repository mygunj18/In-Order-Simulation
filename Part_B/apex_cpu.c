/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "apex_cpu.h"

#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
  switch (stage->opcode)
  {
  case OPCODE_ADD:
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->rs2);
    break;
  }
  case OPCODE_ADDL:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->imm);
    break;
  }
  case OPCODE_SUBL:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->imm);
    break;
  }
  case OPCODE_SUB:
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->rs2);
    break;
  }
  case OPCODE_MUL:
  case OPCODE_DIV:
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->rs2);
    break;
  }
  case OPCODE_LDI:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
    break;
  }
  case OPCODE_STI:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs2, stage->rs1, stage->imm);
    break;
  }
  case OPCODE_CMP:
  {
    printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
    break;
  }
  case OPCODE_OR:
  case OPCODE_AND:
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->rs2);
    break;
  }
  case OPCODE_EXOR:
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->rs2);
    break;
  }
  case OPCODE_NOP:
  case OPCODE_HALT:
  {
    printf("%s", stage->opcode_str);
    break;
  }
  case OPCODE_MOVC:
  {
    printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
    break;
  }
  case OPCODE_LOAD:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
           stage->imm);
    break;
  }
  case OPCODE_STORE:
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
           stage->imm);
    break;
  }
  case OPCODE_BZ:
  {
    printf("%s,#%d ", stage->opcode_str, stage->imm);
    break;
  }
  case OPCODE_BNZ:
  {
    printf("%s,#%d ", stage->opcode_str, stage->imm);
    break;
  }
  case OPCODE_BP:
  {
    printf("%s,#%d ", stage->opcode_str, stage->imm);
    break;
  }
  case OPCODE_BNP:
  {
    printf("%s,#%d ", stage->opcode_str, stage->imm);
    break;
  }
  case OPCODE_JUMP:
  {
    printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
    break;
  }
  }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name,
                    const CPU_Stage *stage)
{
  printf("%-15s: I%d pc(%d)", name, (stage->pc - 4000) / 4, stage->pc);
  print_instruction(stage);
  printf("\t R%d=%d\tR%d=%d \t R%d=%d", stage->rs1, stage->rs1_value, stage->rs2, stage->rs2_value, stage->rd, stage->result_buffer);
  if (stage->isStalled)
  {
    printf(" Stalled");
  }

  printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
  printf("-------------------------------------------\n%s\n-------------------------------------------\n", "STATE OF ARCHITECTURAL REGISTER FILE:");
  for (int i = 0; i < REG_FILE_SIZE; ++i)
  {
    if (cpu->regs[i] || cpu->valid_bit[i])
      printf("|\tR[%d]\t|\tValue=%d \t\t|\tstatus=%s\n", i, cpu->regs[i], cpu->valid_bit[i] ? "invalid" : "valid");
  }

  printf("\n");

  printf("-------------------------------------------\n%s\n-------------------------------------------\n", " STATE OF DATA MEMORY:");
  for (int i = 0; i < DATA_MEMORY_SIZE; ++i)
  {
    if (cpu->data_memory[i])
      printf("|\tMEM[%d]\t|\tData Value=%d\n", i, cpu->data_memory[i]);
  }

  printf("\n");

  printf("-------------------------------------------\n%s\n-------------------------------------------\n", " STATE OF FLAG REGISTERS:");
  printf("Zero_flag = %d\nPositive_flag = %d", cpu->zero_flag, cpu->pos_flag);
}


/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
  APEX_Instruction *current_ins;

  // Checking if fetch stage has instruction and is isStalled or not!
  if (cpu->fetch.has_insn)
  {
    if (!cpu->fetch.isStalled) //if fetch not isStalled
    {
      /* This fetches new branch target instruction from next cycle */
      if (cpu->fetch_from_next_cycle == TRUE)
      {
        cpu->fetch_from_next_cycle = FALSE;

        /* Skip this cycle*/
        return;
      }

      /* Store current PC in fetch latch */
      cpu->fetch.pc = cpu->pc;

      /* Index into code memory using this pc and copy all instruction fields
       * into fetch latch  */
      current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
      strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
      cpu->fetch.opcode = current_ins->opcode;
      cpu->fetch.rd = current_ins->rd;
      cpu->fetch.rs1 = current_ins->rs1;
      cpu->fetch.rs2 = current_ins->rs2;
      cpu->fetch.imm = current_ins->imm;

      /*to check whether D/RF stage is isStalled or not! */
      if (cpu->decode.isStalled)
      {
        cpu->fetch.isStalled = 1; //stalling fetch stage if decode is isStalled
      }
      else
      {
        /* if not isStalled then update the PC for next instruction*/
        cpu->pc += 4;
        /* and copy data from fetch to D/RF */
        cpu->decode = cpu->fetch;
      }
    }
    else
    {
      if (!cpu->decode.isStalled)
      {
        cpu->fetch.isStalled = 0;
        cpu->pc += 4;
        cpu->decode = cpu->fetch;
      }
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->fetch.has_insn)
    {
      print_stage_content("Instrn at Fetch_Stage-->", &cpu->fetch);
    }

    // Upon encountering HALT stop fetching new instructions
    if ((cpu->fetch.opcode == OPCODE_HALT) || (cpu->fetch.opcode == OPCODE_HALT && !cpu->fetch.isStalled))
    {
      cpu->fetch.has_insn = FALSE;
    }
  }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
  if (cpu->decode.has_insn)
  {
    int New_rs1 = 0;
    int New_rs2 = 0;
    //Read operands from register file based on the instruction type
    switch (cpu->decode.opcode)
    {
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_OR:
    case OPCODE_AND:
    case OPCODE_EXOR:
    {
      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      if(cpu->valid_bit[cpu->decode.rs2]){ //if src2 not avaiable then
        New_rs2=1;
      } else{New_rs2=0;}

      // Default unisStalled
      cpu->decode.isStalled = 0;

      // if some previos instruction has marked the destination register invalid there is a posibility that you can get the value in the forwarded register
      // if there is any forwarded sourse register in the file
      if (New_rs1)  //if src1 is not avaiable then
      {
        //if your previous intruction was load and your current instruction is dependent on load then you stall
        // because load cannot produce result until mem stage. So, you stall and send 1 cycle bubble in the pipeline
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {

          cpu->decode.isStalled = 1; //If yes to above conditions then stall the decode
        }
        else //if src1 is avilable
        {
          // if not then you pick from the data from forward register file
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          if (!strcmp(cpu->decode.opcode_str, "STORE")!= 0  && strcmp(cpu->decode.opcode_str, "CMP") != 0)
          {
            cpu->valid_bit[cpu->decode.rd] = 1;
            cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
            cpu->execute = cpu->decode;
          }
        }
      }
      else
      {
        // if there is no forwarded register than you can pick from register file
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        if (strcmp(cpu->decode.opcode_str, "STORE") != 0 && strcmp(cpu->decode.opcode_str, "CMP") != 0)
        {
          cpu->valid_bit[cpu->decode.rd] = 1;
          cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }

      // Same comments for src2
      if (New_rs2)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs2)
        {

          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs2_value = cpu->forwardedDataBuffer[cpu->decode.rs2];
          if (!cpu->decode.isStalled && strcmp(cpu->decode.opcode_str, "STORE") != 0 && strcmp(cpu->decode.opcode_str, "CMP") != 0)
          {
            cpu->valid_bit[cpu->decode.rd] = 1;
            cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
            cpu->execute = cpu->decode;
          }
        }
      }
      else
      {
        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        if (!cpu->decode.isStalled && strcmp(cpu->decode.opcode_str, "STORE") != 0 && strcmp(cpu->decode.opcode_str, "CMP") != 0)
        {
          cpu->valid_bit[cpu->decode.rd] = 1;
          cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }
      cpu->execute = cpu->decode; /* Copy data from decode latch to execute latch*/
      break;
    }

    case OPCODE_ADDL: //1 src and 1dest and 1 literal
    case OPCODE_SUBL:
    {

      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      // Default unisStalled
      cpu->decode.isStalled = 0;

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->valid_bit[cpu->decode.rd] = 1;
          cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->valid_bit[cpu->decode.rd] = 1;
        cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
        cpu->execute = cpu->decode;
      }
      break;
    }

    case OPCODE_LOAD:
    {

      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      // Default unisStalled
      cpu->decode.isStalled = 0;

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->valid_bit[cpu->decode.rd] = 1;
          cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->valid_bit[cpu->decode.rd] = 1;
        cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
        cpu->execute = cpu->decode;
      }
      break;
    }

    case OPCODE_CMP:
    case OPCODE_STORE:
    {
      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      if(cpu->valid_bit[cpu->decode.rs2]){ //if src2 not avaiable then
        New_rs2=1;
      } else{New_rs2=0;}

      cpu->decode.isStalled = 0; //Default un Stalled

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->execute = cpu->decode;
      }

      if (New_rs2)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs2)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs2_value = cpu->forwardedDataBuffer[cpu->decode.rs2];
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        cpu->execute = cpu->decode;
      }


      break;
    }

    case OPCODE_BP:
    case OPCODE_BNP:
    case OPCODE_BZ:
    case OPCODE_BNZ:
    case OPCODE_NOP:
    {
      /* above instructions doesn't have register operands */
      cpu->execute = cpu->decode;
      break;
    }

    case OPCODE_LDI:
    {
      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      if(cpu->valid_bit[cpu->decode.rs2]){ //if src2 not avaiable then
        New_rs2=1;
      } else{New_rs2=0;}

      // by default un Stalled
      cpu->decode.isStalled = 0;

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->valid_bit[cpu->decode.rd] = 1;
          cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->valid_bit[cpu->decode.rd] = 1;
        cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
        cpu->execute = cpu->decode;
      }
      break;
    }

    case OPCODE_STI:
    {
      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      if(cpu->valid_bit[cpu->decode.rs2]){ //if src2 not avaiable then
        New_rs2=1;
      } else{New_rs2=0;}

      // Default unisStalled
      cpu->decode.isStalled = 0;

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->valid_bit[cpu->decode.rs1] = 1; //setting dirty bit for 1st source register
          //cpu->valid_bit[cpu->decode.rs2] = 1; //setting dirty bit for 2nd source register
          cpu->fdata[cpu->decode.rs1] = cpu->decode.pc;
          //cpu->fdata[cpu->decode.rs2] = cpu->decode.pc;
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->valid_bit[cpu->decode.rs1] = 1; //setting dirty bit for 1st source register
        //cpu->valid_bit[cpu->decode.rs2] = 1; //setting dirty bit for 2nd source register
        cpu->fdata[cpu->decode.rs1] = cpu->decode.pc;
        //cpu->fdata[cpu->decode.rs2] = cpu->decode.pc;
        cpu->execute = cpu->decode;
      }

      if (New_rs2)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs2)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs2_value = cpu->forwardedDataBuffer[cpu->decode.rs2];
          cpu->valid_bit[cpu->decode.rs2] = 1; //setting dirty bit for 2nd source register
          cpu->fdata[cpu->decode.rs2] = cpu->decode.pc;          
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        cpu->valid_bit[cpu->decode.rs2] = 1; //setting dirty bit for 2nd source register
        cpu->fdata[cpu->decode.rs2] = cpu->decode.pc;
        cpu->execute = cpu->decode;
      }
      break;
    }

  case OPCODE_MOVC: //dest but no source
    {
      if (!cpu->decode.isStalled) //if decode is not stalled
      { 
        cpu->valid_bit[cpu->decode.rd] = 1;
        cpu->fdata[cpu->decode.rd] = cpu->decode.pc;
        //cpu->decode.isStalled = 1; //stall decode

        /* Copy data from decode latch to execute latch*/
        cpu->execute = cpu->decode;
      }
     else {
      cpu->decode.isStalled=1;  
        }
    break;
    }

    case OPCODE_JUMP:
    {
      // if we dont find sr/dest register and if we have load prior to the current ins then we stall the current ins
      if(cpu->valid_bit[cpu->decode.rs1]){ //if src1 not avaiable then
        New_rs1=1;
      }else{New_rs1=0;}

      if (New_rs1)
      {
        if ((strcmp(cpu->execute.opcode_str, "LOAD") == 0 || strcmp(cpu->execute.opcode_str, "LDI") == 0) && cpu->execute.rd == cpu->decode.rs1)
        {
          cpu->decode.isStalled = 1;
        }
        else
        {
          cpu->decode.rs1_value = cpu->forwardedDataBuffer[cpu->decode.rs1];
          cpu->execute = cpu->decode;
        }
      }
      else
      {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        cpu->execute = cpu->decode; 
      }
      break;
    }

    case OPCODE_HALT:
    {
      cpu->decode.isStalled = 0; //If remains in stall then un stall
      /* Copy data from decode latch to execute latch*/
      cpu->execute = cpu->decode;
      cpu->decode.has_insn = FALSE;
      break;
    }

      //Switch case ends
    }

    if (!cpu->decode.isStalled) //when decode is not stalled
    {
      /* Copy data from decode latch to execute latch*/
      cpu->decode.has_insn = TRUE;
      cpu->execute = cpu->decode;
      cpu->decode.has_insn = FALSE;
    }
    else //when decode is stalled
    {
      cpu->execute = cpu->decode;
      //cpu->execute.opcode = NOP;
      //strcpy(cpu->execute.opcode_str, " ");
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Instrn at Decode/RF_Stage-->", &cpu->decode);
    }
  }
}

/*
     * Execute Stage of APEX Pipeline
     *
     * Note: You are free to edit this function according to your implementation
     */
static void
APEX_execute(APEX_CPU *cpu)
{
  if (cpu->execute.has_insn)
  {
    /* Execute logic based on instruction type */
    switch (cpu->execute.opcode)
    {
    case OPCODE_ADD:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_ADDL: //Setting flag as its execution takes place in in EX stage
    {
      cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_SUB:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_SUBL:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_MUL:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_AND:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value & cpu->execute.rs2_value;
      /* Set the zero flag based on the result buffer */
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_OR:
    { //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_EXOR:
    {
      //Setting flag as its execution takes place in in EX stage
      cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_LOAD:
    {
      //As its execution takes place in memeory and not in EX stage
      cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
      break;
    }

    case OPCODE_BZ: //set branch if not set before. Upon again encountered it will flush the instructions
    {               //Branch if not Zero
      if (cpu->zero_flag == TRUE)
      {
        /* Calculate new PC, and send it to fetch unit */
        cpu->pc = cpu->execute.pc + cpu->execute.imm;

        /* Since we are using reverse callbacks for pipeline stages, 
             * this will prevent the new instruction from being fetched in the current cycle*/
        cpu->fetch_from_next_cycle = TRUE;

        /* Flush previous stages */
    
        cpu->decode.has_insn = FALSE;

        /* Make sure fetch stage is enabled to start fetching from new PC */
        cpu->fetch.has_insn = TRUE;
      }
      break;
    }

    case OPCODE_BNZ: //set branch if not set before. Upon again encountered it will flush the instructions
    {                //Branch if not Zero
      if (cpu->zero_flag == FALSE)
      {
        /* Calculate new PC, and send it to fetch unit */
        cpu->pc = cpu->execute.pc + cpu->execute.imm;

        /* Since we are using reverse callbacks for pipeline stages, 
             * this will prevent the new instruction from being fetched in the current cycle*/
        cpu->fetch_from_next_cycle = TRUE;

        /* Flush previous stages */
        cpu->decode.has_insn = FALSE;

        /* Make sure fetch stage is enabled to start fetching from new PC */
        cpu->fetch.has_insn = TRUE;
      }
      break;
    }

    case OPCODE_BP: //set branch if not set before. Upon again encountered it will flush the instructions
    {               //Branch if Positive
      if (cpu->pos_flag == TRUE)
      {
        /* Calculate new PC, and send it to fetch unit */
        cpu->pc = cpu->execute.pc + cpu->execute.imm;

        /* Since we are using reverse callbacks for pipeline stages, 
             * this will prevent the new instruction from being fetched in the current cycle*/
        cpu->fetch_from_next_cycle = TRUE;

        /* Flush previous stages */
        cpu->decode.has_insn = FALSE;

        /* Make sure fetch stage is enabled to start fetching from new PC */
        cpu->fetch.has_insn = TRUE;
      }
      break;
    }

    case OPCODE_BNP: //set branch if not set before. Upon again encountered it will flush the instructions
    {                //Branch if not Positive
      if (cpu->pos_flag == FALSE)
      {
        /* Calculate new PC, and send it to fetch unit */
        cpu->pc = cpu->execute.pc + cpu->execute.imm;

        /* Since we are using reverse callbacks for pipeline stages, 
             * this will prevent the new instruction from being fetched in the current cycle*/
        cpu->fetch_from_next_cycle = TRUE;

        /* Flush previous stages */
        cpu->decode.has_insn = FALSE;
        /* Make sure fetch stage is enabled to start fetching from new PC */
        cpu->fetch.has_insn = TRUE;
      }
      break;
    }

    case OPCODE_MOVC:
    {
      //It does not execute anything just move literal to specified destination
      cpu->execute.result_buffer = cpu->execute.imm;
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      if (cpu->execute.result_buffer == 0)
     {   
      cpu->zero_flag = TRUE;
     } 
     else 
     {
     cpu->zero_flag = FALSE;
     }
      break;
    }

    case OPCODE_LDI:
    {
      cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
      cpu->execute.resetting_buffer = cpu->execute.rs1_value + 4;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->zero_flag = FALSE;
      }
      break;
    }

    case OPCODE_STI:
    {
      cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
      cpu->execute.resetting_buffer = cpu->execute.rs1_value + 4;
      /* Set the zero flag based on the result buffer */
      if (cpu->execute.result_buffer == 0)
      {
        cpu->zero_flag = TRUE;
      }
      else
      {
        cpu->pos_flag = TRUE;
      }
      break;
    }

    case OPCODE_NOP:
    { //no part in execution
      break;
    }

    case OPCODE_JUMP:
    { //It adds the literal to the source register address and then flushes out all other instruction in pipeline and fetches instruction to thw address where jump jumped to
      cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
      cpu->fetch.has_insn = TRUE;
      cpu->fetch_from_next_cycle = TRUE;
      cpu->decode.has_insn = FALSE;
      break;
    }

    case OPCODE_CMP:
    { //Compares the src registers in execute stage and sets flag accordingly
      if (cpu->execute.rs1_value == cpu->execute.rs2_value)
      {
        cpu->zero_flag = TRUE;
      }
      else{
        cpu->zero_flag = FALSE;
      }
      if(cpu->execute.rs1_value > cpu->execute.rs2_value)
                        {
                             cpu->pos_flag = TRUE;
                        }
                        else
                        {
                             cpu->pos_flag = FALSE;
                        }
      break;
    }
    
    case OPCODE_HALT:
    { 
      cpu->memory = cpu->execute;
      cpu->execute.has_insn = FALSE;
      break;
    }

    case OPCODE_STORE:
    { //Store will store the addition of src register and lietral in mem
      cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
      break;
    }
    }
   
    /* Copy data from execute latch to memory latch*/
    cpu->execute.has_insn = TRUE;
    cpu->memory = cpu->execute;
    cpu->execute.has_insn = FALSE;

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Instrn at Execute_STAGE-->", &cpu->execute);
    }
  }
}

/*
     * Memory Stage of APEX Pipeline
     *
     * Note: You are free to edit this function according to your implementation
     */
static void
APEX_memory(APEX_CPU *cpu)
{
  if (cpu->memory.has_insn)
  {
    switch (cpu->memory.opcode)
    {
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_JUMP:
    case OPCODE_EXOR:
    case OPCODE_CMP:
    case OPCODE_MOVC:
    case OPCODE_BZ:
    case OPCODE_BNZ:
    case OPCODE_BP:
    case OPCODE_BNP:
    case OPCODE_ADDL:
    case OPCODE_SUBL:
    case OPCODE_HALT:
    {
      /* No work for ADD */
      break;
    }

    case OPCODE_LOAD:
    {
      /* Read from data memory */
      cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
      cpu->forwardedDataBuffer[cpu->memory.rd] = cpu->memory.result_buffer;
      break;
    }

    case OPCODE_STORE:
    {
      /* write data to memory */
      cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
      break;
    }

    case OPCODE_LDI:
    {
      /* Read from data memory */
      cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
      cpu->forwardedDataBuffer[cpu->execute.rd] = cpu->execute.result_buffer;
      break;
    }

    case OPCODE_STI:
    {
      /* write data to memory */
      cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs2_value;
      break;
    }
    }

    /* Copy data from memory latch to writeback latch*/
    cpu->writeback = cpu->memory;
    cpu->memory.has_insn = FALSE;

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Instrn at MEMORY_STAGE-->", &cpu->memory);
    }
  }
}


/*
     * Writeback Stage of APEX Pipeline
     *
     * Note: You are free to edit this function according to your implementation
     */
static int
APEX_writeback(APEX_CPU *cpu)
{
  if (cpu->writeback.has_insn)
  {
    /* Write result to register file based on instruction type */
    switch (cpu->writeback.opcode)
    {
    case OPCODE_ADD:
    case OPCODE_ADDL:
    case OPCODE_SUB:
    case OPCODE_SUBL:
    case OPCODE_MUL:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_EXOR:
    case OPCODE_LOAD:
    case OPCODE_MOVC:
    {
      cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
      if (cpu->fdata[cpu->writeback.rd] == cpu->writeback.pc)
      {
        cpu->valid_bit[cpu->writeback.rd] = 0;
      }
      break;
    }

    case OPCODE_LDI:
    {
      cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
      cpu->regs[cpu->writeback.rs1_value] = cpu->writeback.resetting_buffer;
      if (cpu->fdata[cpu->writeback.rd] == cpu->writeback.pc)
      {
        cpu->valid_bit[cpu->writeback.rd] = 0;
      }
      if (cpu->fdata[cpu->writeback.rs1] == cpu->writeback.pc)
      {
        cpu->valid_bit[cpu->writeback.rs1] = 0;
      }
      break;
    }
    
    case OPCODE_STI:
    {
     cpu->regs[cpu->writeback.rs1_value] = cpu->writeback.resetting_buffer;
      if (cpu->fdata[cpu->writeback.rs1] == cpu->writeback.pc)
      {
        cpu->valid_bit[cpu->writeback.rs1] = 0;
        cpu->valid_bit[cpu->writeback.rs2] = 0;
      }
      break;
    }
    case OPCODE_CMP:
    case OPCODE_NOP:
    case OPCODE_JUMP:
    {
      break;
    }
    case OPCODE_BZ:
    case OPCODE_BNZ:
    case OPCODE_BP:
    case OPCODE_BNP:
    {
      break;
    }

    case OPCODE_HALT:
    {
      if(!cpu->writeback.isStalled)
    {          
      cpu->writeback.has_insn = FALSE;
    }
    else{ /* Stop the APEX simulator */ 
      cpu->decode.isStalled = 0; //If remains in stall then un stall and then HALT the CPU
      cpu->writeback.has_insn = FALSE;
    }
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Instrn at WRITEBACK_Stage-->", &cpu->writeback);
    }

      return TRUE;
    }
  }
    cpu->insn_completed++;
    cpu->writeback.has_insn = FALSE;

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Instrn at WRITEBACK_Stage-->", &cpu->writeback);
    }

    /* Default */
    return 0;
  }
}


/*
     * This function creates and initializes APEX cpu.
     *
     * Note: You are free to edit this function according to your implementation
     */
APEX_CPU *
APEX_cpu_init(const char *filename, const char *op, const int no_of_cycles)
{
  int i;
  APEX_CPU *cpu;

  if (!filename && !op && !no_of_cycles) //to check valid function and cylces added
  {
    return NULL;
  }

  cpu = calloc(1, sizeof(APEX_CPU));

  if (!cpu)
  {
    return NULL;
  }

  if (strcmp(op, "showmem") != 0)
  {
    cpu->opCycles = no_of_cycles;
  }
  else
  {
    cpu->memLoc = no_of_cycles;
    cpu->showMem = 1;
  }

  // when asked for simulation set simulate property otherwise display*/
  if (strcmp(op, "simulate") == 0)
  {
    cpu->simulate = 1;
  }
  else
  {
    cpu->simulate = 0;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
  memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
  cpu->single_step = 0;
  if (strcmp(op, "single_step") == 0)
  {
    cpu->single_step = ENABLE_SINGLE_STEP;
  }

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
  if (!cpu->code_memory)
  {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES && !cpu->simulate)
  {
    fprintf(stderr,
            "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
    fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
           "imm");

    for (i = 0; i < cpu->code_memory_size; ++i)
    {
      printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
             cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
    }
  }

  /* To start fetch stage */
  cpu->fetch.has_insn = TRUE;
  return cpu;
}

/*
     * APEX CPU simulation loop
     *
     * Note: You are free to edit this function according to your implementation
     */
void APEX_cpu_run(APEX_CPU *cpu)
{
  char user_prompt_val;

  while (TRUE) //Running CPU till clock <= to code memory size*/
  {
    if (ENABLE_DEBUG_MESSAGES && !cpu->simulate) //if not simulate
    {
      printf("--------------------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------------------\n");
    }

    if (APEX_writeback(cpu) || (cpu->clock == cpu->opCycles && !cpu->showMem))
    //when Halt stop instruction
    {
      /* Halt in writeback stage */
      printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
      break;
    }

    APEX_memory(cpu);
    APEX_execute(cpu);
    APEX_decode(cpu);
    APEX_fetch(cpu);

    //to display content of register file at each stage for single_step in print_reg_file(cpu);
    if (cpu->single_step)
    {
      print_reg_file(cpu);
      printf("Press any key to advance CPU Clock or <q> to quit:\n");
      scanf("%c", &user_prompt_val);

      if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
      {
        printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
        break;
      }
    }

    cpu->clock++;
  }
}

/*
     * This function deallocates APEX CPU.
     *
     * Note: You are free to edit this function according to your implementation
     */
void APEX_cpu_stop(APEX_CPU *cpu)
{
  if (!cpu->single_step)
    free(cpu->code_memory);
  free(cpu);
}