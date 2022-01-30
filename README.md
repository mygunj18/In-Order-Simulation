# APEX Pipeline Simulator (Asynchronous Pipeline EXecution) In-order 5 stage system simulator

 - This c code consists of 5 Stages of instruction processing by processor: Fetch -> Decode -> Execute -> Memory -> Writeback with latency of 1 cycle
 - Implemented Data forwarding logic in Part B and without forwarding logic in Part A
 - Included logic for instructions ADD,SUB,MUL,DIV,AND,OR,EXOR,MOVC,LOAD,STORE,BZ,BNZ,HALT,ADDL,SUBL,JUMP,LDI,STI,NOP,BP,BNP,CMP
 - Also included logic for HALT instruction. On fetching HALT, fetch will stop fetching new instructions and one by one it will flow through every stage and then stop the simulator

## Program modules:

 - 'apex_cpu.h' - Declarations of Data structures used
 - 'apex_cpu.c' - Implementation of APEX cpu
 - 'apex_macros.h' 
 - 'main.c' - Main function which calls APEX CPU interface
 - 'file_parser.c' - Functions to parse input file
 - 'input.asm' - Sample input file
 - 'Makefile'

## Steps to compile and run

 make
```
 ./apex_sim input.asm <input_file_name>
