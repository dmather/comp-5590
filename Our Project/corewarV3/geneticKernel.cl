#include "sharedredcode.h"

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

// I think this works now
void do_one_step(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS], int i)
{
  int prog_counter;
  int pointer_value;
  int index_a;
  int value_a;
  int index_b;
  int value_b;

  prog_counter = pcs[i][c_proc[i]];

  // cerr << "Executing: " << mem[prog_counter] << endl;

  if(n_proc[i] > 0){

    // current process of program i is c_proc[i]
    // current program counter is pcs[i][c_proc[i]]
    // cell to execute is mem[pcs[i][c_proc[i]]]

    // find indices of the memory cells that the instruction is working with

    switch(mem[prog_counter].mode_A){
    case IMM: // get the value from the current memory location
      index_a = prog_counter;
      value_a = mem[index_a].arg_A;
      break;
    case DIR: // get the value pointed to by the current memory location relative to the current memory location
      index_a = (prog_counter + mem[prog_counter].arg_A) % MEMORY_SIZE;
      value_a = mem[index_a].arg_B;
      break;
    case IND: // follow the pointer pointed to by the current memory location
      pointer_value = (prog_counter + mem[prog_counter].arg_A) % MEMORY_SIZE;
      index_a = (pointer_value + mem[pointer_value].arg_B) % MEMORY_SIZE;
      value_a = mem[index_a].arg_B;
      break;
    //default:
      //cerr << "Bad addressing mode!" << endl;
    }

    switch(mem[pcs[i][c_proc[i]]].mode_B){
    case IMM: // get the value from the current memory location
      index_b = prog_counter;
      value_b = mem[index_b].arg_B;
      break;
    case DIR: // get the value pointed to by the current memory location relative to the current memory location
      index_b = (prog_counter + mem[prog_counter].arg_B) % MEMORY_SIZE;
      value_b = mem[index_b].arg_B;
      break;
    case IND: // follow the pointer pointed to by the current memory location
      pointer_value = (prog_counter + mem[prog_counter].arg_B) % MEMORY_SIZE;
      index_b = (pointer_value + mem[pointer_value].arg_B) % MEMORY_SIZE;
      value_b = mem[index_b].arg_B;
      break;
    //default:
      //cerr << "Bad addressing mode!" << endl;
    }

    // do the instruction
    switch(mem[pcs[i][c_proc[i]]].code){
    case DAT:
      n_proc[i] = 0;
      break;
    case MOV:
      mem[(prog_counter + value_b) % MEMORY_SIZE] = mem[(prog_counter + value_a) % MEMORY_SIZE];
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
      if(value_a > 0){
        mem[index_b].arg_B /= value_a;
      } else {
        n_proc[i] = 0;
      }
      break;
    case MOD:
      if(value_a > 0){
        mem[index_b].arg_B %= value_a;
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
      if(mem[index_b].arg_B == 0){
        pcs[i][c_proc[i]] += value_a;
        pcs[i][c_proc[i]]--;
        pcs[i][c_proc[i]] %= MEMORY_SIZE;
      }
      break;
    case DJZ:
      if(mem[index_b].arg_B != 0){
        pcs[i][c_proc[i]] += value_a;
        pcs[i][c_proc[i]]--;
        pcs[i][c_proc[i]] %= MEMORY_SIZE;
      }
      break;
    case CMP:
      if(mem[index_b].arg_B == value_a){
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

__kernel void test(__global int *output)
{
    int gid = get_global_id(0);
    output[gid] = 5*gid;
}

void clear_cw(memory_cell mem[MEMORY_SIZE],
              int pcs[N_PROGRAMS][MAX_PROCESSES],
              int c_proc[N_PROGRAMS],
              int n_proc[N_PROGRAMS])
{
  int i, j;
  memory_cell blank;

  blank.code = DAT;
  blank.arg_A = 0;
  blank.arg_B = 0;
  blank.mode_A = DIR;
  blank.mode_B = DIR;

  // Clear Memory
  for(i=0;i<MEMORY_SIZE;i++){
    mem[i] = blank;
  }

  for(i=0;i<N_PROGRAMS;i++){
    c_proc[i] = 0;
    n_proc[i] = 0;
    for(j=0;j<MAX_PROCESSES;j++){
      pcs[i][j] = 0;
    }
  }

  return;
}

void clear_survivals(int survivals[POPULATION_SIZE])
{
  int i;

  for(i=0;i<POPULATION_SIZE;i++){
    survivals[i] = 0;
  }

  return;
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

void load_cw(memory_cell mem[MEMORY_SIZE],
             int pcs[N_PROGRAMS][MAX_PROCESSES],
             int c_proc[N_PROGRAMS],
             int n_proc[N_PROGRAMS],
             int starting_locations[N_PROGRAMS],
             memory_cell program[N_PROGRAMS][MAX_PROGRAM_LENGTH])
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
void run_cw(memory_cell mem[MEMORY_SIZE],
            int pcs[N_PROGRAMS][MAX_PROCESSES],
            int c_proc[N_PROGRAMS],
            int n_proc[N_PROGRAMS],
            int *duration)
{
  int time_step;
  int i, j;

  time_step = 1;
  while(time_step <= MAX_STEPS && survivor_count(n_proc) > 2){
    //    cerr << "time_step = " << time_step << endl;
    for(i=0;i<N_PROGRAMS;i++){
      do_one_step(mem,pcs,c_proc,n_proc,i);
    }
    time_step++;
  }

  duration = time_step;

  return;
}

int max_t_length(int t_lengths[N_TOURNAMENTS])
{
  int max = 0;
  int i;

  for(i=0;i<N_TOURNAMENTS;i++){
    if(t_lengths[i] > max){
      max = t_lengths[i];
    }
  }

  return max;
}
