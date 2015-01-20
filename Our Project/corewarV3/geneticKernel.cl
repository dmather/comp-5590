#include "sharedredcode.h"
// This is an PRNG library from:
// http://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html#overview
#include "cl/mwc64x.cl"

void do_one_step(int mem[MEMORY_SIZE],
                 int pcs[N_PROGRAMS][MAX_PROCESSES],
                 int c_proc[N_PROGRAMS],
                 int n_proc[N_PROGRAMS], int i);

int survivor_count(int n_proc[MAX_PROCESSES]);

void record_survivals(int survivals[POPULATION_SIZE],
                      int n_process[N_PROGRAMS],
                      int selected[N_PROGRAMS]);


memory_cell convertIntToMemoryCell(int mem);

int convertMemoryCellToInt(memory_cell mem);

void select_programs(int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
                     int progs[N_PROGRAMS][MAX_PROGRAM_LENGTH],
                     int selected[N_PROGRAMS],
                     mwc64x_state_t rng)
{
    int i,j;
    int r;

    for(i=0;i<N_PROGRAMS;i++){
        //r = rand() % POPULATION_SIZE;
        r = MWC64X_NextUint(&rng) % POPULATION_SIZE;
        selected[i] = r;
        for(j=0;j<MAX_PROGRAM_LENGTH;j++){
            progs[i][j] = pop[r][j];
        }
    }
    return;
}

// Maybe we could input the memoryCell as a pointer to save on memory
memory_cell convertIntToMemoryCell(int mem)
{
    memory_cell tCell;
    tCell.code = mem & 0b11110000000000000000000000000000;
    tCell.code>>=28;
    tCell.mode_A = mem & 0b00001100000000000000000000000000;
    tCell.mode_A>>=26;
    tCell.arg_A = mem & 0b00000011111111111100000000000000;
    tCell.arg_A>>=14;
    tCell.mode_B = mem & 0b00000000000000000011000000000000;
    tCell.mode_B>>=12;
    tCell.arg_B = mem & 0b00000000000000000000111111111111;
    tCell.arg_B>>=0;

    return tCell;
}

int convertMemoryCellToInt(memory_cell mem)
{
    int tMem;
    int tInst;
    int tMode_A;
    int tArg_A;
    int tMode_B;
    int tArg_B;

    tInst = mem.code;
    tInst<<=28;
    tMode_A = mem.mode_A;
    tMode_A<<=26;
    tArg_A = mem.arg_A;
    tArg_A<<=14;
    tMode_B = mem.mode_B;
    tMode_B<<=12;
    tArg_B = mem.arg_B;
    tArg_B<<=0;

    tMem = tInst | tMode_A | tArg_A | tMode_B | tArg_B;
    return tMem;
}

void load_cw(int mem[MEMORY_SIZE],
             int pcs[N_PROGRAMS][MAX_PROCESSES],
             int c_proc[N_PROGRAMS],
             int n_proc[N_PROGRAMS],
             int starting_locations[N_PROGRAMS],
             int program[N_PROGRAMS][MAX_PROGRAM_LENGTH])
{
    int i,j;
    int s;

    for(i=0;i<N_PROGRAMS;i++){
        c_proc[i]=0;
        n_proc[i]=1;
        pcs[i][0]=starting_locations[i];
        s = starting_locations[i];
        for(j=0;j<MAX_PROGRAM_LENGTH;j++){
            mem[s+j] = program[i][j];
        }
    }
    return;
}

// I think this works now
// Survivors have a non-zero entry in n_proc
/*
void run_cw(memory_cell mem[MEMORY_SIZE],
			int pcs[N_PROGRAMS][MAX_PROCESSES],
			int c_proc[N_PROGRAMS],
			int n_proc[N_PROGRAMS],
			int duration)
{
	int time_step;
	int i, j;
	time_step = 1;
	while(time_step <= MAX_STEPS && survivor_count(n_proc) > 2){
		// cerr << "time_step = " << time_step << endl;
		for(i=0;i<N_PROGRAMS;i++){
			do_one_step(mem,pcs,c_proc,n_proc,i);
		}
		time_step++;
	}
	duration = time_step;
	return;
}
*/

// I think this works now
void do_one_step(int mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS], int i)
{
    int prog_counter;
    int pointer_value;
    int index_a;
    int value_a;
    int index_b;
    int value_b;
    memory_cell tCell;
    memory_cell myCell;

    prog_counter = pcs[i][c_proc[i]];
    // cerr << "Executing: " << mem[prog_counter] << endl;
    if(n_proc[i] > 0){
        // current process of program i is c_proc[i]
        // current program counter is pcs[i][c_proc[i]]
        // cell to execute is mem[pcs[i][c_proc[i]]]
        // find indices of the memory cells that the instruction is working with
        tCell = convertIntToMemoryCell(mem[prog_counter]);
        switch(tCell.mode_A){
        case IMM: // get the value from the current memory location
            index_a = prog_counter;
            value_a = convertIntToMemoryCell(mem[index_a]).arg_A;
            break;
        case DIR: // get the value pointed to by the current memory location relative to the current memory location
            index_a = (prog_counter + tCell.arg_A) % MEMORY_SIZE;
            value_a = convertIntToMemoryCell(mem[index_a]).arg_B;
            break;
        case IND: // follow the pointer pointed to by the current memory location
            pointer_value = (prog_counter + tCell.arg_A) % MEMORY_SIZE;
            index_a = (pointer_value + convertIntToMemoryCell(mem[pointer_value]).arg_B) % MEMORY_SIZE;
            value_a = convertIntToMemoryCell(mem[index_a]).arg_B;
            break;
        //default:
            //cerr << "Bad addressing mode!" << endl;
        }
        tCell = convertIntToMemoryCell(mem[pcs[i][c_proc[i]]]);
        switch(tCell.mode_B){
        case IMM: // get the value from the current memory location
            index_b = prog_counter;
            value_b = convertIntToMemoryCell(mem[index_b]).arg_B;
            break;
        case DIR: // get the value pointed to by the current memory location relative to the current memory location
            index_b = (prog_counter + convertIntToMemoryCell(mem[prog_counter]).arg_B) % MEMORY_SIZE;
            value_b = convertIntToMemoryCell(mem[index_b]).arg_B;
            break;
        case IND: // follow the pointer pointed to by the current memory location
            pointer_value = (prog_counter + convertIntToMemoryCell(mem[prog_counter]).arg_B) % MEMORY_SIZE;
            index_b = (pointer_value + convertIntToMemoryCell(mem[pointer_value]).arg_B) % MEMORY_SIZE;
            value_b = convertIntToMemoryCell(mem[index_b]).arg_B;
            break;
        //default:
            //cerr << "Bad addressing mode!" << endl;
        }
        // do the instruction
        tCell = convertIntToMemoryCell(mem[pcs[i][c_proc[i]]]);
        switch(tCell.code){
        case DAT:
            n_proc[i] = 0;
            break;
        case MOV:
            mem[(prog_counter + value_b) % MEMORY_SIZE] = mem[(prog_counter + value_a) % MEMORY_SIZE];
            break;
        case ADD:
            myCell = convertIntToMemoryCell(mem[index_b]);
            myCell.arg_B += value_a;
            myCell.arg_B %= MEMORY_SIZE;
            mem[index_b] = convertMemoryCellToInt(myCell);
            break;
        case SUB:
            myCell = convertIntToMemoryCell(mem[index_b]);
            myCell.arg_B -= value_a;
            myCell.arg_B %= MEMORY_SIZE;
            mem[index_b] = convertMemoryCellToInt(myCell);
            break;
        case MUL:
            myCell = convertIntToMemoryCell(mem[index_b]);
            myCell.arg_B *= value_a;
            myCell.arg_B %= MEMORY_SIZE;
            mem[index_b] = convertMemoryCellToInt(myCell);
            break;
        case DIV:
            if(value_a > 0){
                myCell = convertIntToMemoryCell(mem[index_b]);
                myCell.arg_B /= value_a;
                mem[index_b] = convertMemoryCellToInt(myCell);
            } else {
                n_proc[i] = 0;
            }
            break;
        case MOD:
            if(value_a > 0){
                myCell = convertIntToMemoryCell(mem[index_b]);
                myCell.arg_B %= value_a;
                mem[index_b] = convertMemoryCellToInt(myCell);
            } else {
                n_proc[i] = 0;
            }
            break;
        case JMP:
            pcs[i][c_proc[i]] += value_a;
            pcs[i][c_proc[i]]--;
            pcs[i][c_proc[i]] %= MEMORY_SIZE;
            break;
        case JMZ:
            if(convertIntToMemoryCell(mem[index_b]).arg_B == 0){
                pcs[i][c_proc[i]] += value_a;
                pcs[i][c_proc[i]]--;
                pcs[i][c_proc[i]] %= MEMORY_SIZE;
            }
            break;
        case DJZ:
            if(convertIntToMemoryCell(mem[index_b]).arg_B != 0){
                pcs[i][c_proc[i]] += value_a;
                pcs[i][c_proc[i]]--;
                pcs[i][c_proc[i]] %= MEMORY_SIZE;
            }
            break;
        case CMP:
            if(convertIntToMemoryCell(mem[index_b]).arg_B == value_a){
                pcs[i][c_proc[i]]++;
            }
            break;
        case SPL:
            if(n_proc[i] < MAX_PROCESSES){
                pcs[i][n_proc[i]] = (pcs[i][c_proc[i]] + value_a) % MEMORY_SIZE;
                n_proc[i]++;
            }
            break;
        case NOP:
            break;
        //default:
            //cerr << "Bad instruction" << endl;
        }
        // update program counter
        pcs[i][c_proc[i]]++;
        pcs[i][c_proc[i]] %= MEMORY_SIZE;
        c_proc[i]++;
        if(n_proc[i] != 0) c_proc[i] %= n_proc[i]; else c_proc[i] = 0;
    }
    return;
}

// Count how many have processes still running
int survivor_count(int n_proc[MAX_PROCESSES]){
	int count = 0;

	for(int i = 0; i < MAX_PROCESSES; i++){
		if(n_proc[i] > 0){
			count++;
		}
	}

	return count;
}

void record_survivals(int survivals[POPULATION_SIZE],
					  int n_process[N_PROGRAMS],
					  int selected[N_PROGRAMS])
{
	int i;
	for(i=0;i<N_PROGRAMS;i++){
		if(n_process[i] > 0){
			survivals[selected[i]]++;
		}
	}

	return;
}

__kernel void test(__global int *test,
                   __global int *starts,
                   __global int *population,
                   __global int *survivals,
                   __global int *tournament_lengths)
{
    // Get the work unit ID
    int gid = get_global_id(0);
    int lid = get_local_id(0);
    mwc64x_state_t rng;
    MWC64X_SeedStreams(&rng, 2, 4);

    // Simulator vars
    int current_programs[N_PROGRAMS][MAX_PROGRAM_LENGTH];
    int memory[MEMORY_SIZE];
    int selected[N_PROGRAMS];
    int program_counters[N_PROGRAMS][MAX_PROCESSES];
    int curr_process[N_PROGRAMS];
    int n_processes[N_PROGRAMS];

    __private int rand = MWC64X_NextUint(&rng);

    select_programs(population, current_programs, selected, rng);
    load_cw(memory, program_counters, curr_process, n_processes, &starts[gid],
            current_programs);

    int time_step = 1;
    while(time_step <= MAX_STEPS && survivor_count(n_processes) > 2){
        for(int i = 0; i < N_PROGRAMS; i++){
            do_one_step(memory, program_counters,
                        curr_process, n_processes, i);
        }
        time_step++;
        tournament_lengths[gid] = time_step;
    }
    record_survivals(survivals, n_processes, selected);
    test[gid] = time_step;
}
