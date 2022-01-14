/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc != 4)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file> <Operation> <No. of cycles>\n", argv[0]);
        exit(1);
    }
    int n=atoi(argv[3]);
    cpu = APEX_cpu_init(argv[1] , argv[2], n); // for input file, simulate/display/single_step, number of cycles*/);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }

    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}