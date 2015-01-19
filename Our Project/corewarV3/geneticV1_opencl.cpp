// Genetic Algorithm Draft 1
// using
// Core Wars Draft 3
// December 2014
// Charley Hepler

// Changes from Core Wars Draft 3
// Added functions: 
//                  clear_cw
//                  load_cw
//                  run_cw              -- added a parameter just 'cause I was curious about output
//                  generate_program    -- uniformly at random with a fixed length
//                  generate_population -- calls generate program
//                  generate_starts     -- for fixed length programs
//                  select_programs
//                  clear_survivals
//                  breed               -- too simple: just cross over and one point mutation
//                  show_part           -- just to see that something happens
//
// Changed functions:
//                  do_one_step: removed debugging print statements
//                               added check for number of processes to SPT instruction
//                               added zero check on DIV and MOD
//                  main: to be the genetic algorithm -- NOT DONE YET
//
// No longer using:
//                  init_cw
//                  show_results -- was just for debugging, replaced by show_part

#include "sharedredcode.h"
#include "geneticV1_opencl.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <bitset>
#include <CL/cl.h>

using namespace std;

ostream& operator<<(ostream& outs, const memory_cell& cell);
istream& operator>>(istream& ins, memory_cell& cell);

int main()
{

	//memory_cell memory[MEMORY_SIZE];
	//int program_counters[N_PROGRAMS][MAX_PROCESSES];
	//int n_processes[N_PROGRAMS];
	//int curr_process[N_PROGRAMS];

    int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH];
    int chldrn[POPULATION_SIZE][MAX_PROGRAM_LENGTH];

    //memory_cell population[POPULATION_SIZE][MAX_PROGRAM_LENGTH];
	memory_cell children[POPULATION_SIZE][MAX_PROGRAM_LENGTH];
	int survivals[POPULATION_SIZE];
	//int selected[N_PROGRAMS];

	//memory_cell current_programs[N_PROGRAMS][MAX_PROGRAM_LENGTH];
	//int starts[N_PROGRAMS];

	int generation;
	int i,j,k;

	int tournament_lengths[N_TOURNAMENTS];
	int max_tournament_length[MAX_GENERATIONS];

	// Vars needed for GPU kernel
	int gpu_starts[N_TOURNAMENTS][N_PROGRAMS];

	// OpenCL variables
	cl_context context = 0;
	cl_command_queue commandQueue = 0;
	cl_program  program = 0;
	cl_device_id device = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	cl_int errNum;
	int test[N_TOURNAMENTS];

	srand(1234); // set a specific seed for replicability in debugging

	// generate initial population
    //generate_population(population);
    gen_int_population(pop);

	cout << "Before: " << endl;
    show_part_int(pop);
    //show_part(population);

	// OpenCL host code
	context = CreateContext();
	if(context == NULL)
	{
		printf("Failed to create OpenCL context.\n");
	}

	commandQueue = CreateCommandQueue(context, &device);
	if(commandQueue == NULL)
	{
		// Cleanup method
		return 1;
	}

	char name[2048] = "";
	size_t workItems[3];
	size_t paramSize = -1;
	//unsigned long localMemSize = -1;
	// This needs to be adjusted
	size_t globalWorkSize[1] = { N_TOURNAMENTS };
	size_t localWorkSize[1] = { 1 };

	errNum = clGetDeviceInfo(device, CL_DEVICE_NAME,
		sizeof(name),
		&name, &paramSize);
	printf("Requested %lu bytes\n", paramSize);
	printf("Device name: %s.\n", name);
	if(errNum != CL_SUCCESS)
	{
		printf("Error getting device info: %d\n", errNum);
	}
	errNum = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES,
		sizeof(workItems),
		&workItems, &paramSize);
	printf("Requested %lu bytes\n", paramSize);
	printf("Max work items: %lu\n", workItems[0]);
	if(errNum != CL_SUCCESS)
	{
		printf("Error getting device info: %d\n", errNum);
	}
	
	program = CreateProgram(context, device, "geneticKernel.cl");
	if(program == NULL)
	{
		return 1;
	}
	// End OpenCL host code

    for(generation = 0; generation < MAX_GENERATIONS; generation++){
		// run tournaments
		clear_survivals(survivals);
		for(i = 0; i < N_TOURNAMENTS; i++){
			generate_starts(gpu_starts[i]);
			//select_programs(population,current_programs,selected);
			//load_cw(memory,program_counters,curr_process,
			//        n_processes,starts,current_programs);
			//run_cw(memory,program_counters,curr_process,n_processes,tournament_lengths[i]);
			//record_survivals(survivals,n_processes,selected);
		}

		kernel = clCreateKernel(program, "test", NULL);
		if(kernel == NULL)
		{
			printf("Failed to create kernel.\n");
		}

		//if(!CreateMemObjects(context, memObjects, population, test, gpu_starts))
		//{
		//	printf("Failed to create mem objects\n");
		//	return 1;
		//}

		cl_int errNo;
		// Example buffer objects
		memObjects[0] = clCreateBuffer(context,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(int) * (POPULATION_SIZE * MAX_PROGRAM_LENGTH),
                pop, &errNo);
		cout << "Cl Error: " << errNo << endl;
		memObjects[7] = clCreateBuffer(context,
				CL_MEM_WRITE_ONLY,
				sizeof(int) * N_TOURNAMENTS, NULL, &errNo);
		cout << "Cl Error: " << errNo << endl;
		memObjects[8] = clCreateBuffer(context,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				sizeof(int) * (N_TOURNAMENTS * N_PROGRAMS),
				gpu_starts, &errNo);
		cout << "Cl Error: " << errNo << endl;
		memObjects[1] = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * POPULATION_SIZE, survivals, &errNo);
		cout << "Cl Error: " << errNo << endl;
		memObjects[2] = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * N_TOURNAMENTS, tournament_lengths, &errNo);
		cout << "Cl Error: " << errNo << endl;

		errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[7]);
		errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[8]);
		errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[0]);
		errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[1]);
		errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &memObjects[2]);
		
		if(errNum != CL_SUCCESS)
		{
			printf("Error setting kernel arguments. %d\n", errNum);
		}

        size_t kernel_work_group_size;
		clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t), &kernel_work_group_size, NULL);
		cout << "Kernel Work Group Size: " << kernel_work_group_size << endl;

		if(checkError(errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
			globalWorkSize, localWorkSize, 0,
			NULL, NULL)))
		{
			printf("Error queueing the kernel for execution. %d\n", errNum);
			continue;
		}

		clFinish(commandQueue);

		for(int i = 0; i < N_TOURNAMENTS; i++)
		{
			test[i] = -1;
		}

		if(checkError(errNum = clEnqueueReadBuffer(commandQueue, memObjects[7], CL_TRUE, 0,
												   sizeof(int) * N_TOURNAMENTS, test, 0, NULL, NULL)))
		{
			cout << "Read Buffer Error: " << errNum << endl;
			continue;
		}
		if(checkError(errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE, 0,
												   sizeof(int) * POPULATION_SIZE, survivals, 0, NULL, NULL)))
		{
			cout << "Read Buffer Error: " << errNum << endl;
			continue;
		}
		if(checkError(errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0,
			sizeof(int) * N_TOURNAMENTS, tournament_lengths, 0, NULL,
			NULL)))
		{
			cout << "Read Buffer Error: " << errNum << endl;
			continue;
		}

		for(int i = 0; i < N_TOURNAMENTS; i++)
		{
			cout << "Random Number: " << test[i] << endl;
			cout << "Survivals: " << survivals[i] << endl;
		}

		cout << "Generation: " << generation << endl;

		cout << "Max Tournament Length: " << max_t_length(tournament_lengths) << endl;
		max_tournament_length[generation] = max_t_length(tournament_lengths);

		// variation here??? are the tournaments doing anything?
		//    if(generation % 10 == 0){
		//      for(j=0;j<N_TOURNAMENTS;j++){
		//	cout << tournament_lengths[j] << endl;
		//      }
		//      cout << endl;
		//    }

		// breed victors
		// the survivals array has a count of how many times each program won
		// use that to select which programs get to breed
        j = 0;
		for(int i = 0; i < POPULATION_SIZE; i++){
			// find two parents for child
			while(survivals[j] == 0){
				j++;
				j %= POPULATION_SIZE;
			}
			k = j + 1;
			k %= POPULATION_SIZE;
			while(survivals[k] == 0){
				k++;
				k %= POPULATION_SIZE;
			}
            //breed(population[j],population[k],children[i]);
            breed_int(pop[j],pop[k],chldrn[i]);

			j = k + 1;
			j %= POPULATION_SIZE;
		}

		// children becomes next generation
		for(int i = 0; i < POPULATION_SIZE; i++){
			for(j = 0; j < MAX_PROGRAM_LENGTH; j++){
                pop[i][j] = chldrn[i][j];
			}
		}
		// Release Memory Objects (Maybe to save some kernel memory
		for(int i = 0; i < 9; i++)
		{
			clReleaseMemObject(memObjects[i]);
		}
		clReleaseKernel(kernel);
		//clFinish(commandQueue);
		//Sleep(1000);
	}
	cout << "After: " << endl;
    show_part_int(pop);

	//  for(i = 0; i < 10; i++){
	//    cout << "Generation " << i << " max run time = " << max_tournament_length[i] << endl;
	//  }
	//  cout << "." << endl;
	//  cout << "." << endl;
	//  cout << "." << endl;
	//  for(i = MAX_GENERATIONS - 10; i < MAX_GENERATIONS; i++){
	//    cout << "Generation " << i << " max run time = " << max_tournament_length[i] << endl;
	//  }

	//clFinish(commandQueue);

	// Hacky cleanup
	clFinish(commandQueue);
	//for(unsigned int i = 0; i<9; i++)
	//{
	//	clReleaseMemObject(memObjects[i]);
	//}
	//clReleaseKernel(kernel);
	//clReleaseProgram(program);
	//clReleaseContext(context);
	//clReleaseCommandQueue(commandQueue);


	cin.get();

	return 0;
}

bool checkError(cl_int errNum)
{
	if(errNum == CL_SUCCESS)
		return false;
	else
		return true;
}

cl_context CreateContext(void)
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;

	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if(errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		printf("Failed to find any OpenCL platforms.\n");
		return NULL;
	}

	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)firstPlatformId,
		0
	};

	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
		NULL, NULL, &errNum);
	if(errNum != CL_SUCCESS)
	{
		printf("could not create GPU context, trying CPU...\n");
		context = clCreateContextFromType(contextProperties,
			CL_DEVICE_TYPE_CPU, NULL, NULL,
			&errNum);
		if(errNum != CL_SUCCESS)
		{
			printf("Failed to create an OpenCL GPU or CPU context.\n");
			return NULL;
		}
	}
	return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;

	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL,
		&deviceBufferSize);
	if(errNum != CL_SUCCESS)
	{
		printf("Failed called to clGetContextInf(..., CL_CONTEXT_DEVICES, ...)\n");
		return NULL;
	}

	if(deviceBufferSize <= 0)
	{
		printf("No devices available.\n");
		return NULL;
	}

	devices = new cl_device_id[deviceBufferSize/sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize,
		devices, NULL);
	if(errNum != CL_SUCCESS)
	{
		printf("Failed to get device IDs\n");
		delete[] devices;
		return NULL;
	}

	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if(commandQueue == NULL)
	{
		printf("Failed to create command Queue for device 0\n");
		return NULL;
	}

	*device = devices[0];
	delete [] devices;
	return commandQueue;
}

cl_program CreateProgram(cl_context context, cl_device_id device,
						 const char* fileName)
{
	cl_int errNum;
	cl_program program;

	ifstream kernelFile(fileName, ios::in);
	if(!kernelFile.is_open())
	{
		printf("Failed to open file for reading: %s\n", fileName);
		return NULL;
	}

	ostringstream oss;
	oss << kernelFile.rdbuf();

	string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1, (const char**)&srcStr,
		NULL, NULL);
	if(program == NULL)
	{
		printf("Failed to create CL program from source.\n");
		return NULL;
	}

	// Fourth argument contains a string for opencl kernel preprocessor
	// search directory, this allows us to use includes.
	errNum = clBuildProgram(program, 0, NULL, "-I ./", NULL, NULL);
	if(errNum != CL_SUCCESS)
	{
		char buildLog[16384];
		errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);
		cout << "clGetProgramBuildInfo: " << errNum << endl;
		cerr << "Error in kernel: " << endl;
		cout << buildLog << endl;
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}

bool CreateMemObjects(cl_context context,
					  cl_mem memObjects[9],
                      int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
					  int test[N_TOURNAMENTS],
					  int starts[N_TOURNAMENTS][N_PROGRAMS])
{
	cl_int errNo;
	// Example buffer objects
	memObjects[0] = clCreateBuffer(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(*pop), 
		pop, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[7] = clCreateBuffer(context,
		CL_MEM_WRITE_ONLY,
		sizeof(int) * N_TOURNAMENTS, NULL, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[8] = clCreateBuffer(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(*starts),
		starts, &errNo);
	cout << "Cl Error: " << errNo << endl;

	return true;
}


void clear_survivals(int survivals[POPULATION_SIZE])
{
	int i;

	for(i=0;i<POPULATION_SIZE;i++){
		survivals[i] = 0;
	}

	return;
}

void gen_int_program(int prog[MAX_PROGRAM_LENGTH])
{
    int i;
    for(i = 0; i<MAX_PROGRAM_LENGTH; i++) {
        // There are 13 instruction which fit into 4-bits
        // Each variable is temporary so we can bitshift them the
        // right direction.
        unsigned tInst = (instruction) (rand() % N_OPERATIONS);
        unsigned tMode_A = (addressing) (rand() % N_MODES);
        int tArg_A = rand() % MEMORY_SIZE;
        unsigned tMode_B = (addressing) (rand() % N_MODES);
        int tArg_B = rand() % MEMORY_SIZE;

        // We bitshift by the according number of bits we've given each
        // type 4-bits for instruction, 2-bits for modes, and 12-bits for
        // arguments. All numbers are unsigned except arguments which should
        // be twos complement and are signed so they need an additional bit.
        // e.g in our argument we have 11 data bits and 1 sign bit.
        tInst<<=28;
        tMode_A<<=26;
        tArg_A<<=14;
        tMode_B<<=12;
        tArg_B<<=0;

        prog[i] = prog[i] | tInst | tMode_A | tArg_A | tMode_B | tArg_B;
        // We just need to make sure that the data makes sense.
        cout << (bitset<32>) prog[i] << endl;
    }
}

// generates a program with random instructions
void generate_program(memory_cell prog[MAX_PROGRAM_LENGTH])
{
	int i;
	for(i=0;i<MAX_PROGRAM_LENGTH;i++){
		prog[i].code = (instruction) (rand() % N_OPERATIONS);
		prog[i].arg_A = rand() % MEMORY_SIZE;
		prog[i].mode_A = (addressing) (rand() % N_MODES);
		prog[i].arg_B = rand() % MEMORY_SIZE;
		prog[i].mode_B = (addressing) (rand() % N_MODES);
	}
}

// generates an array of programs
void generate_population(memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH])
{
	int i;

	for(i=0;i<POPULATION_SIZE;i++){
		generate_program(pop[i]);
	}
}

void gen_int_population(int pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH])
{
    int i;

    for(i = 0; i<POPULATION_SIZE; i++) {
        gen_int_program(pop[i]);
    }
}

void generate_starts(int starts[N_PROGRAMS])
{
	int i;
	int programs_to_place;
	int space_needed_for_programs;
	int start_spaces_available;
	int offset;

	starts[0] = 0;
	for(i=1;i<N_PROGRAMS;i++){
		programs_to_place = N_PROGRAMS - i;
		space_needed_for_programs = (programs_to_place + 1) * MAX_PROGRAM_LENGTH;
		start_spaces_available = MEMORY_SIZE - starts[i-1] - space_needed_for_programs;
		if(start_spaces_available != 0){
			offset = rand() % start_spaces_available;
		} else {
			offset = 0;
		}
		starts[i] = starts[i-1] + MAX_PROGRAM_LENGTH + offset;
	}
}

void breed(memory_cell father[MAX_PROGRAM_LENGTH],
		   memory_cell mother[MAX_PROGRAM_LENGTH],
		   memory_cell child[MAX_PROGRAM_LENGTH])
{
	int cut_location;
	int point_mutation;
	int i;

	cut_location = (rand() % (MAX_PROGRAM_LENGTH - 2)) + 1;
	point_mutation = rand() % MAX_PROGRAM_LENGTH;

	for(i=0;i<cut_location;i++){
		child[i] = father[i];
	}
	for(i=cut_location;i<MAX_PROGRAM_LENGTH;i++){
		child[i] = mother[i];
	}

	child[point_mutation].code = (instruction) (rand() % N_OPERATIONS);
	child[point_mutation].arg_A = rand() % MEMORY_SIZE;
	child[point_mutation].mode_A = (addressing) (rand() % N_MODES);
	child[point_mutation].arg_B = rand() % MEMORY_SIZE;
	child[point_mutation].mode_B = (addressing) (rand() % N_MODES);

	return;
}


void breed_int(int father[MAX_PROGRAM_LENGTH],
               int mother[MAX_PROGRAM_LENGTH],
               int child[MAX_PROGRAM_LENGTH])
{
    int cut_location;
    int point_mutation;
    int i;

    unsigned tInst;
    unsigned tMode_A;
    int tArg_A;
    unsigned tMode_B;
    int tArg_B;

    cut_location = (rand() % (MAX_PROGRAM_LENGTH -2)) + 1;
    point_mutation = rand() % MAX_PROGRAM_LENGTH;

    for(i = 0; i<cut_location; i++)
    {
        child[i] = father[i];
    }
    for(i = cut_location; i<MAX_PROGRAM_LENGTH; i++)
    {
        child[i] = mother[i];
    }

    // Generate our new values
    tInst = (instruction) (rand() % N_OPERATIONS);
    tMode_A = (addressing) (rand() % N_MODES);
    tArg_A = rand() % MEMORY_SIZE;
    tMode_B = (addressing) (rand() % N_MODES);
    tArg_B = rand() % MEMORY_SIZE;

    // Shift to correct locations
    tInst<<=28;
    tMode_A<<=26;
    tArg_A<<=14;
    tMode_B<<=12;
    tArg_B<<=0;

    child[point_mutation] = child[point_mutation] | tInst | tMode_A | tArg_A |
            tMode_B | tArg_B;
    cout << (bitset<32>) child[point_mutation] << endl;
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

// really just for debugging
void show_part(memory_cell population[POPULATION_SIZE][MAX_PROGRAM_LENGTH])
{
	int i;

	for(i=0;i<10;i++){
		cout << population[0][i] << endl;
	}

	return;
}

void show_part_int(int population[POPULATION_SIZE][MAX_PROGRAM_LENGTH])
{
    int i;
    for(i = 0; i<10; i++) {
        int memObj = population[0][i];
        unsigned int tInst;
        unsigned int tMode_A;
        int tArg_A;
        unsigned int tMode_B;
        int tArg_B;

        // Binary and the memObj with a bitmask to get the correct var.
        tInst = memObj &    0b11110000000000000000000000000000;
        tInst>>=28;
        tMode_A = memObj &  0b00001100000000000000000000000000;
        tMode_A>>=26;
        tArg_A = memObj &   0b00000011111111111100000000000000;
        tArg_A>>=14;
        tMode_B = memObj &  0b00000000000000000011000000000000;
        tMode_B>>=12;
        tArg_B = memObj &   0b00000000000000000000111111111111;
        tArg_B>>=0;

        /*
        cout << "Inst: " << tInst;
        cout << ", Mode A: " << tMode_A;
        cout << ", Arg A: " << tArg_A;
        cout << ", Mode B: " << tMode_B;
        cout << ", Arg B: " << tArg_B << endl;
        */

        switch(tInst) {
        case DAT: cout << "DAT"; break;
        case MOV: cout << "MOV"; break;
        case ADD: cout << "ADD"; break;
        case SUB: cout << "SUB"; break;
        case MUL: cout << "MUL"; break;
        case DIV: cout << "DIV"; break;
        case MOD: cout << "MOD"; break;
        case JMP: cout << "JMP"; break;
        case JMZ: cout << "JMZ"; break;
        case DJZ: cout << "DJZ"; break;
        case CMP: cout << "CMP"; break;
        case SPL: cout << "SPL"; break;
        case NOP: cout << "NOP"; break;
        default: cout << endl << "*** ERRR: no such instruction ***" << endl;
        }
        cout << " ";
        switch(tMode_A) {
        case IMM: cout << "#"; break;
        case DIR: cout << "$"; break;
        case IND: cout << "@"; break;
        default: cout << endl << "*** ERROR: no such addressing mode" << endl;
        }
        cout << tArg_A << " ";
        switch(tMode_B) {
        case IMM: cout << "#"; break;
        case DIR: cout << "$"; break;
        case IND: cout << "@"; break;
        default: cout << endl << "*** ERROR: no such addressing mode" << endl;
        }
        cout << tArg_B << " " << endl;
    }
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

ostream& operator<<(ostream& outs, const memory_cell& cell)
{
	switch(cell.code){
	case DAT: outs << "DAT"; break;
	case MOV: outs << "MOV"; break;
	case ADD: outs << "ADD"; break;
	case SUB: outs << "SUB"; break;
	case MUL: outs << "MUL"; break;
	case DIV: outs << "DIV"; break;
	case MOD: outs << "MOD"; break;
	case JMP: outs << "JMP"; break;
	case JMZ: outs << "JMZ"; break;
	case DJZ: outs << "DJZ"; break;
	case CMP: outs << "CMP"; break;
	case SPL: outs << "SPL"; break;
	case NOP: outs << "NOP"; break;
	default: outs << endl << "*** ERROR: no such instruction ***" << endl;
	}
	outs << " ";
	switch(cell.mode_A){
	case IMM: outs << "#"; break;
	case DIR: outs << "$"; break;
	case IND: outs << "@"; break;
	default: outs << endl << "*** ERROR: no such addressing mode" << endl;
	}
	outs << cell.arg_A << " ";
	switch(cell.mode_B){
	case IMM: outs << "#"; break;
	case DIR: outs << "$"; break;
	case IND: outs << "@"; break;
	default: outs << endl << "*** ERROR: no such addressing mode" << endl;
	}
	outs << cell.arg_B;

	return outs;
}
