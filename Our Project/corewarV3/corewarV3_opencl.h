#ifndef COREWARV3_OPENCL_H
#define COREWARV3_OPENCL_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include "sharedredcode.h"

using namespace std;

void init_cw(memory_cell mem[MEMORY_SIZE],
             int pcs[N_PROGRAMS][MAX_PROCESSES],
             int c_proc[N_PROGRAMS],
             int n_proc[N_PROGRAMS]);

void run_cw(memory_cell mem[MEMORY_SIZE],
            int pcs[N_PROGRAMS][MAX_PROCESSES],
            int c_proc[N_PROGRAMS],
            int n_proc[N_PROGRAMS]);

void do_one_step(memory_cell mem[MEMORY_SIZE],
                 int pcs[N_PROGRAMS][MAX_PROCESSES],
                 int c_proc[N_PROGRAMS],
                 int n_proc[N_PROGRAMS]);

void show_results(int n_proc[N_PROGRAMS]);

int survivor_count(int n_proc[MAX_PROCESSES]);

ostream& operator<<(ostream& outs, const memory_cell& cell);
istream& operator>>(istream& ins, memory_cell& cell);

#endif // COREWARV3_OPENCL_H
