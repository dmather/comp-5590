#ifndef GENETICV1_OPENCL_H
#define GENETICV1_OPENCL_H

#include "sharedredcode.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

bool checkError(cl_int errNum);

void clear_cw(memory_cell mem[MEMORY_SIZE],
              int pcs[N_PROGRAMS][MAX_PROCESSES],
              int c_proc[N_PROGRAMS],
              int n_proc[N_PROGRAMS]);

void clear_survivals(int survivals[POPULATION_SIZE]);

void select_programs(memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
                     memory_cell progs[N_PROGRAMS][MAX_PROGRAM_LENGTH],
                     int selected[N_PROGRAMS]);

void load_cw(memory_cell mem[MEMORY_SIZE],
             int pcs[N_PROGRAMS][MAX_PROCESSES],
             int c_proc[N_PROGRAMS],
             int n_proc[N_PROGRAMS],
             int starting_locations[N_PROGRAMS],
             memory_cell program[N_PROGRAMS][MAX_PROGRAM_LENGTH]);

void run_cw(memory_cell mem[MEMORY_SIZE],
            int pcs[N_PROGRAMS][MAX_PROCESSES],
            int c_proc[N_PROGRAMS],
            int n_proc[N_PROGRAMS],
            int& duration);

void do_one_step(memory_cell mem[MEMORY_SIZE],
                 int pcs[N_PROGRAMS][MAX_PROCESSES],
                 int c_proc[N_PROGRAMS],
                 int n_proc[N_PROGRAMS],
                 int prog);

int max_t_length(int t_lengths[N_TOURNAMENTS]);

void generate_program(memory_cell prog[MAX_PROGRAM_LENGTH]);

void gen_int_population(int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);

void gen_int_program(int prog[MAX_PROGRAM_LENGTH]);

void generate_population(memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);
void generate_starts(int starts[N_PROGRAMS]);
void record_survivals(int survials[POPULATION_SIZE],
                      int n_process[N_PROGRAMS],
                      int selected[N_PROGRAMS]);

void breed(memory_cell father[MAX_PROGRAM_LENGTH],
           memory_cell mother[MAX_PROGRAM_LENGTH],
           memory_cell child[MAX_PROGRAM_LENGTH]);

void breed_int(int father[MAX_PROGRAM_LENGTH],
               int mother[MAX_PROGRAM_LENGTH],
               int child[MAX_PROGRAM_LENGTH]);

void show_part(memory_cell population[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);

void show_part_int(int population[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);

int survivor_count(int n_proc[MAX_PROCESSES]);

cl_context CreateContext(void);

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);

cl_program CreateProgram(cl_context context, cl_device_id device,
                         const char* fileName);

// Figure out how many objects need to be in memObjects
//bool CreateMemObjects(cl_context context, cl_mem memObjects[4], int test[1000]);

bool CreateMemObjects(cl_context context,
                      cl_mem memObjects[8],
int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
int test[N_TOURNAMENTS],
int starts[N_TOURNAMENTS][N_PROGRAMS]);

#endif // GENETICV1_OPENCL_H
