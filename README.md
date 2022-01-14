# In-Order-Simulation


#Apex Simulator

## Details:

 - Considering 5 stage APEX In-order Pipeline
 - All the stages have latency of one cycle
 - Considering single functional unit in Execute stage which perform all the arithmetic and logic operations
 - Logic to check data dependencies is added
 - Includes logic for 'ADD','ADDL','SUB','SUBL','MUL','STORE','LOAD','STI','LDI','JUMP','EXOR','OR','AND','NOP','CMP','MOVC','BP','BNP','BZ', 'BNZ' and 'HALT' instructions
 - Includes logic for On fetching 'HALT' instruction, fetch stage stop fetching new instructions and when 'HALT' instruction is in commit stage, simulation stops
 - Consists of simulation instructions showing memory details, architectural details, instructions in every stage as per cycle, single_Step, dispaly, showmem
 - Part A consists of code without data forwarding logic of in-order Pipelined instructions
 - Part B code consists of code with logic of data forwarding of in-ordder pipelined instructions
 - Consists of test cases for Part A and B and simulation conditions required

## Files in Part A and Part B folder contains:

 - 'Makefile'
 - 'file_parser.c' - Functions to parse input file
 - 'apex_cpu.h' - Data structures declarations
 - 'apex_cpu.c' - Implementation of APEX cpu
 - 'apex_macros.h' - Macros used in the implementation
 - 'main.c' - Main function which calls APEX CPU interface
 - 'input.asm' - Sample input file

## How to compile and run

 Go to terminal and particular folder
```

 Step 1)  make
 Step 2)  ./apex_sim input.asm single_step 70
```
