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
void generate_population(memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);
void generate_starts(int starts[N_PROGRAMS]);
void record_survivals(int survials[POPULATION_SIZE],
                      int n_process[N_PROGRAMS],
                      int selected[N_PROGRAMS]);

void breed(memory_cell father[MAX_PROGRAM_LENGTH],
           memory_cell mother[MAX_PROGRAM_LENGTH],
           memory_cell child[MAX_PROGRAM_LENGTH]);

void show_part(memory_cell population[POPULATION_SIZE][MAX_PROGRAM_LENGTH]);

int survivor_count(int n_proc[MAX_PROCESSES]);

cl_context CreateContext(void);

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);

cl_program CreateProgram(cl_context context, cl_device_id device,
                         const char* fileName);

// Figure out how many objects need to be in memObjects
//bool CreateMemObjects(cl_context context, cl_mem memObjects[4], int test[1000]);

bool CreateMemObjects(cl_context context,
                      cl_mem memObjects[8],
memory_cell mem[N_TOURNAMENTS][MEMORY_SIZE],
int pcs[N_TOURNAMENTS][N_PROGRAMS][MAX_PROCESSES],
int c_proc[N_TOURNAMENTS][N_PROGRAMS],
int n_proc[N_TOURNAMENTS][N_PROGRAMS],
int tournament_lengths[N_TOURNAMENTS],
int survivals[N_TOURNAMENTS][POPULATION_SIZE],
int selected[N_TOURNAMENTS][N_PROGRAMS],
int test[N_TOURNAMENTS]);

#endif // GENETICV1_OPENCL_H
