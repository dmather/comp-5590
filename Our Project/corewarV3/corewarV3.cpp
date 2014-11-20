// Core Wars Draft 3
// November 2014
// Charley Hepler

#include <iostream>
#include <iomanip>
#include "sharedredcode.h"
#include "redcode_fileio.h"

using namespace std;

void init_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS]);
void run_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS]);
void do_one_step(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS], int prog);
void show_results(int n_proc[N_PROGRAMS]);
int survivor_count(int n_proc[MAX_PROCESSES]);

int main()
{
  memory_cell memory[MEMORY_SIZE];
  int program_counters[N_PROGRAMS][MAX_PROCESSES];
  int n_processes[N_PROGRAMS];
  int curr_process[N_PROGRAMS];
  
  int i,j;

  init_cw(memory, program_counters, curr_process, n_processes);

  cout << "**** Programs loaded ****" << endl;
  for(i = 0; i < 40; i++){
    cout << memory[i] << endl;
  }


  run_cw(memory, program_counters, curr_process, n_processes);

  //  show_results(n_processes);

  return 0;
}


// just sets n_procs to 2 and reads from two files (with file names hard coded)
//
void init_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS])
{
  ifstream file_one("program_one.txt");
  ifstream file_two("program_two.txt");

  int i;
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

  // Load program one
  for(i = 0; i < 5; i++){ // first program starts at location 0
    file_one >> mem[i];
  }
  pcs[0][0] = 0; // first program starts at location 0
  c_proc[0] = 0; // with only one process
  n_proc[0] = 1; 

  // Load program two
  for(i = 20; i < 25; i++){ // second program starts at location 20
    file_two >> mem[i];
  }
  pcs[1][0] = 20; // second program starts at location 20
  c_proc[1] = 0; // with only one process
  n_proc[1] = 1; 

  file_one.close();
  file_two.close();
  
  return;
}

// completely untested ZZZZZ
void run_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS])
{
  int time_step;
  int i, j;

  time_step = 1;
  while(time_step <= MAX_ITERATIONS && survivor_count(n_proc) > 2){
    for(i=0;i<N_PROGRAMS;i++){
      cout << endl << "Doing step " << time_step << " of program " << i << endl;

      // do one step of program i if it is alive
      do_one_step(mem,pcs,c_proc,n_proc,i);

      cout << "Memory contents after step " << time_step << endl;
      for(j = 0; j < MEMORY_SIZE; j++){
	cout << setw(4) << j << ": " << mem[j] << endl;
      }
      cout << endl;
    }
    time_step++;
  }

  return;
}

void do_one_step(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES], int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS], int i)
{
  int prog_counter;
  int pointer_value;
  int index_a;
  int value_a;
  int index_b;
  int value_b;

  prog_counter = pcs[i][c_proc[i]];

  cout << "  program number (i) = " << i << endl;
  cout << "  n_proc[i] = " << n_proc[i] << endl;
  cout << "  c_proc[i] = " << c_proc[i] << endl;
  cout << "  prog_counter = " << prog_counter << endl;
  cout << "Executing: " << mem[pcs[i][c_proc[i]]] << endl;


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
    default:
      cerr << "Bad addressing mode!" << endl;
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
    default:
      cerr << "Bad addressing mode!" << endl;
    }
    
    cout << "  value_a = " << value_a << endl;
    cout << "  index_a = " << index_a << endl;
    cout << "  value_b = " << value_b << endl;
    cout << "  index_b = " << index_b << endl;

    // do the instruction ZZZZZ
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
      mem[index_b].arg_B /= value_a;
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
      pcs[i][n_proc[i]] = (pcs[i][c_proc[i]] + value_a) % MEMORY_SIZE;
      n_proc[i]++;
      break;
    case NOP:
      break;
    default:
      cerr << "Bad instruction" << endl;
    }
    	
    // update program counter
    pcs[i][c_proc[i]]++;
    pcs[i][c_proc[i]] %= MEMORY_SIZE;
    c_proc[i]++;
    if(n_proc[i] != 0) c_proc[i] %= n_proc[i]; else c_proc[i] = 0;
    
  } else {
    cout << "Program " << i << " is dead." << endl;
  }
 
  return;
}

// really just for debugging
void show_results(int n_proc[N_PROGRAMS])
{
  int i;

  for(i=0;i<N_PROGRAMS;i++){
    cout << "Program " << i << ((n_proc[N_PROGRAMS] > 0) ? " survived." : " died.") << endl;
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
