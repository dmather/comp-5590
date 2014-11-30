// Very simple opencl kernel
#include "sharedredcode.h"

/*
#define MEMORY_SIZE 40 // just little for debugging
#define N_PROGRAMS 2 // first just test each instruction
#define MAX_PROCESSES 1000
#define MAX_ITERATIONS 1000

typedef enum instruction {
	DAT, // not executable, values stored in B
	MOV, // copy A to B
	ADD, // add A into B (storing in B)
	SUB, // subtract A from B (storing in B)
	MUL, // multiply A into B (storing in B)
	DIV, // B / A (storing in B)
	MOD, // B mod A (storing in B)
	JMP, // transfer control to address A
	JMZ, // transfer control to address A if B is zero
	DJZ, // decrement B and if B is now 0, transfer control to A
	CMP, // if A != B skip next instruction
	SPL, // split execution into next instruction and instruction at A
	NOP  // do nothing -- used as "punctuation" for genetic algorithm
} instruction;

typedef enum addressing {
	IMM, // immediate (#)
	DIR, // direct ($)
	IND, // indirect (@)
} addressing;

typedef struct memory_cell {
	instruction code;
	int arg_A;
	addressing mode_A;
	int arg_B;
	addressing mode_B;
} memory_cell;
*/

int survivor_count(int n_proc[N_PROGRAMS], int log[10])
{
	__private int count = 0;
	for(__private int i = 0; i < N_PROGRAMS; i++)
	{
		if(n_proc[i] > 0)
		{
			count++;
		}
	}
	log[9] = count;
	return count;
}
void do_one_step(memory_cell mem[MEMORY_SIZE],
				 int pcs[N_PROGRAMS][MAX_PROCESSES],
				 int c_proc[N_PROGRAMS],
				 int n_proc[N_PROGRAMS], int i,
				 int log[2048])
{
	int prog_counter;
	int pointer_value;
	int index_a;
	int value_a;
	int index_b;
	int value_b;
	prog_counter = pcs[i][c_proc[i]];
	log[1] = n_proc[i];
	if(n_proc[i] > 0)
	{
		switch(mem[prog_counter].mode_A) {
		case IMM:
			index_a = prog_counter;
			value_a = mem[index_a].arg_A;
			break;
		case DIR:
			index_a = (prog_counter + mem[prog_counter].arg_A) % MEMORY_SIZE;
			value_a = mem[index_a].arg_B;
			break;
		case IND:
			pointer_value = (prog_counter + mem[prog_counter].arg_A) % MEMORY_SIZE;
			index_a = (pointer_value + mem[pointer_value].arg_B) % MEMORY_SIZE;
			value_a = mem[index_a].arg_B;
			break;
		}
		switch(mem[pcs[i][c_proc[i]]].mode_B) {
		case IMM:
			index_b = prog_counter;
			value_b = mem[index_b].arg_B;
			break;
		case DIR:
			index_b = (prog_counter + mem[prog_counter].arg_B) % MEMORY_SIZE;
			value_b = mem[index_b].arg_B;
			break;
		case IND:
			pointer_value = (prog_counter + mem[prog_counter].arg_B) % MEMORY_SIZE;
			index_b = (pointer_value + mem[pointer_value].arg_B) % MEMORY_SIZE;
			value_b = mem[index_b].arg_B;
			break;
		}
		switch(mem[pcs[i][c_proc[i]]].code) {
		case DAT:
			log[5] = DAT;
			n_proc[i] = 0;
			break;
		case MOV:
			mem[(prog_counter + value_b) %
				MEMORY_SIZE] = mem[(prog_counter + value_a) % MEMORY_SIZE];
			break;
		case ADD:
			mem[index_b].arg_B += value_a;
			mem[index_b].arg_B %= MEMORY_SIZE;
			break;
		case SUB:
			mem[index_b].arg_B -= value_a;
			mem[index_b].arg_B %= MEMORY_SIZE;
			break;
		case MUL:
			mem[index_b].arg_B *= value_a;
			mem[index_b].arg_B %= MEMORY_SIZE;
			break;
		case DIV:
			mem[index_b].arg_B /= value_a;
			//mem[index_b].arg_B %= MEMORY_SIZE;
			break;
		case MOD:
			mem[index_b].arg_B %= value_a;
			break;
		case JMP:
			pcs[i][c_proc[i]] += value_a;
			pcs[i][c_proc[i]]--;
			pcs[i][c_proc[i]] %= MEMORY_SIZE;
			break;
		case JMZ:
			if(mem[index_b].arg_B != 0) {
				pcs[i][c_proc[i]] += value_a;
				pcs[i][c_proc[i]]--;
				pcs[i][c_proc[i]] %= MEMORY_SIZE;
			}
			break;
		case DJZ:
			if(mem[index_b].arg_B != 0) {
				pcs[i][c_proc[i]] += value_a;
				pcs[i][c_proc[i]]--;
				pcs[i][c_proc[i]] %= MEMORY_SIZE;
			}
			break;
		case CMP:
			if(mem[index_b].arg_B == value_a) {
				pcs[i][c_proc[i]]++;
			}
			break;
		case SPL:
			pcs[i][n_proc[i]] = (pcs[i][c_proc[i]] + value_a) % MEMORY_SIZE;
			n_proc[i]++;
			break;
		case NOP:
			break;
		}
		pcs[i][c_proc[i]]++;
		pcs[i][c_proc[i]] %= MEMORY_SIZE;
		c_proc[i]++;
		if(n_proc[i] != 0) c_proc[i] %= n_proc[i]; else c_proc[i] = 0;
		log[0] = 0;
		log[2] = i;
	}
	else
	{
		log[0] = 1;
		log[2] = i;
	}
}
void run_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES],
			int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS], int log[10])
{
	int time_step;
	int i;
	int j;
	time_step = 1;
	while(survivor_count(n_proc, log) > 1 && time_step <= MAX_ITERATIONS)
	{
		log[3] = time_step;
		for(i = 0; i < N_PROGRAMS; i++)
		{
			do_one_step(mem, pcs, c_proc, n_proc, i, log);
		}
		time_step++;
	}
}

/*
__kernel void hello_kernel(__global const float *a,
__global const float *b,
__global float *result)
{
// get work-item id (array index)
int gid = get_global_id(0);

// add a and b and store them in the work-item index in result
result[gid] = a[gid] + b[gid];
}
*/

void clear_log(int log[10])
{
	for(int i; i<10; i++)
	{
		log[i] = -1;
	}
}

__kernel void test(__global memory_cell *mem __attribute__((endian(host))),
				   __global int *pcs __attribute__((endian(host))),
				   __global int *c_proc __attribute__((endian(host))),
				   __global int *n_proc __attribute__((endian(host))),
				   __global int *log __attribute__((endian(host))))
{
	clear_log(log);
	//log[0] = 100;
	//int gid = get_global_id(0);
	//log[gid] = gid;
	//simple_func(n_proc, log);
	//log[9] = n_proc[1];
	//clear_log(log);
	run_cw(mem, pcs, c_proc, n_proc, log);
	//for(int i = 0; i<MEMORY_SIZE; i++){
	//	mem[i].arg_A = 2;
	//}
}
