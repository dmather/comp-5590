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
#include <CL/cl.h>

using namespace std;

ostream& operator<<(ostream& outs, const memory_cell& cell);
istream& operator>>(istream& ins, memory_cell& cell);

int main()
{

	memory_cell memory[MEMORY_SIZE];
	int program_counters[N_PROGRAMS][MAX_PROCESSES];
	int n_processes[N_PROGRAMS];
	int curr_process[N_PROGRAMS];

	memory_cell population[POPULATION_SIZE][MAX_PROGRAM_LENGTH];
	memory_cell children[POPULATION_SIZE][MAX_PROGRAM_LENGTH];
	int survivals[POPULATION_SIZE];
	int selected[N_PROGRAMS];

	memory_cell current_programs[N_PROGRAMS][MAX_PROGRAM_LENGTH];
	int starts[N_PROGRAMS];

	int generation;
	int i,j,k;

	//int tournament_lengths[N_TOURNAMENTS];
	//int max_tournament_length[MAX_GENERATIONS];

	// Vars needed for GPU kernel
	//memory_cell gpu_mem[N_TOURNAMENTS][MEMORY_SIZE];
	//int gpu_pcs[N_TOURNAMENTS][N_PROGRAMS][MAX_PROCESSES];
	//int gpu_c_proc[N_TOURNAMENTS][N_PROGRAMS];
	//int gpu_n_proc[N_TOURNAMENTS][N_PROGRAMS];
	//int gpu_tournament_lengths[N_TOURNAMENTS];
	//int gpu_survivals[N_TOURNAMENTS][POPULATION_SIZE];
	//int gpu_selected[N_TOURNAMENTS][N_PROGRAMS];
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
	generate_population(population);

	cout << "Before: " << endl;
	show_part(population);

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
	unsigned long localMemSize = -1;
	// This needs to be adjusted
	size_t globalWorkSize[1] = { N_TOURNAMENTS };
	size_t localWorkSize[1] = { 1 };

	errNum = clGetDeviceInfo(device, CL_DEVICE_NAME,
		sizeof(name),
		&name, &paramSize);
	printf("Requested %d bytes\n", paramSize);
	printf("Device name: %s.\n", name);
	if(errNum != CL_SUCCESS)
	{
		printf("Error getting device info: %d\n", errNum);
	}
	errNum = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES,
		sizeof(workItems),
		&workItems, &paramSize);
	printf("Requested %d bytes\n", paramSize);
	printf("Max work items: %lu\n", workItems[0]);
	if(errNum != CL_SUCCESS)
	{
		printf("Error getting device info: %d\n", errNum);
	}
	/*
	errNum = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE,
	sizeof(unsigned long),
	&localMemSize, &paramSize);
	printf("Requested %d bytes\n", paramSize);
	printf("Local Memory size: %lu\n", localMemSize);
	if(errNum != CL_SUCCESS)
	{
	printf("Error getting device info: %d\n", errNum);
	}
	*/
	program = CreateProgram(context, device, "geneticKernel.cl");
	if(program == NULL)
	{
		return 1;
	}

	kernel = clCreateKernel(program, "test", NULL);
	if(kernel == NULL)
	{
		printf("Failed to create kernel.\n");
	}

	// End OpenCL host code


	for(generation = 0; generation < MAX_GENERATIONS; generation++){
		// run tournaments
		clear_survivals(survivals);
		for(i = 0; i < N_TOURNAMENTS; i++){
			clear_cw(memory,program_counters,curr_process,n_processes);
			generate_starts(gpu_starts[i]);
			// Copy the starting positions into gpu_starts array.
			//memcpy(gpu_starts[i], starts, sizeof(int) * N_PROGRAMS);
			//select_programs(population,current_programs,selected);
			//load_cw(memory,program_counters,curr_process,
			//        n_processes,starts,current_programs);
			//run_cw(memory,program_counters,curr_process,n_processes,tournament_lengths[i]);
			//record_survivals(survivals,n_processes,selected);
		}
		if(!CreateMemObjects(context, memObjects, population, test, gpu_starts))
		{
			printf("Failed to create mem objects\n");
			return 1;
		}

		errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[7]);
		errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[8]);
		errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[0]);
		//errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
		//errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[3]);
		//errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &memObjects[4]);
		//errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &memObjects[5]);
		//errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &memObjects[6]);
		//errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &memObjects[7]);
		if(errNum != CL_SUCCESS)
		{
			printf("Error setting kernel arguments. %d\n", errNum);
		}

		size_t kernel_work_group_size;
		clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t), &kernel_work_group_size, NULL);
		cout << "Kernel Work Group Size: " << kernel_work_group_size << endl;

		errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
			globalWorkSize, localWorkSize, 0,
			NULL, NULL);
		if(errNum != CL_SUCCESS)
		{
			printf("Error queueing the kernel for execution. %d\n", errNum);
		}

		errNum = clFinish(commandQueue);
		if(errNum != CL_SUCCESS)
		{
			printf("Error with clFinish command. %d\n", errNum);
		}

		// Read back the buffers
		/*
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[0], CL_TRUE, 0,
		sizeof(gpu_mem), gpu_mem, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE, 0,
		sizeof(gpu_pcs), gpu_pcs, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0,
		sizeof(gpu_c_proc), gpu_c_proc, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[3], CL_TRUE, 0,
		sizeof(gpu_n_proc), gpu_n_proc, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[4], CL_TRUE, 0,
		sizeof(gpu_tournament_lengths), gpu_tournament_lengths, 0,
		NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[5], CL_TRUE, 0,
		sizeof(gpu_survivals), gpu_survivals, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[6], CL_TRUE, 0,
		sizeof(gpu_selected), gpu_selected, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		//
		*/
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[7], CL_TRUE, 0,
			sizeof(*test), test, 0, NULL, NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[8], CL_TRUE, 0,
			sizeof(*gpu_starts), starts, 0, NULL,
			NULL);
		cout << "Read Buffer Error: " << errNum << endl;
		//tournament_lengths = &gpu_tournament_lengths;
		//memcpy(tournament_lengths, gpu_tournament_lengths,
		//       sizeof(gpu_tournament_lengths));


		for(i=0; i<N_TOURNAMENTS;i++)
		{
			cout << "Random Number: " << test[i] << endl;
			cout << "Program starts: " << gpu_starts[i][1] << endl;
		}

		cout << "Iteration: " << generation << endl;
		//cout << generation << endl;

		//max_tournament_length[generation] = max_t_length(tournament_lengths);

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
		/*
		j = 0;
		for(i=0;i<POPULATION_SIZE;i++){
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
		breed(population[j],population[k],children[i]);

		j = k + 1;
		j %= POPULATION_SIZE;
		}

		// children becomes next generation
		for(i=0;i<POPULATION_SIZE;i++){
		for(j=0;j<MAX_PROGRAM_LENGTH;j++){
		population[i][j] = children[i][j];
		}
		}
		*/
	}
	cout << "After: " << endl;
	show_part(population);
	//cout << "Sizeof gpu_mem: " << sizeof(gpu_mem)/1024 << "KiB" << endl;
	//cout << "Sizeof gpu_c_proc: " << sizeof(gpu_c_proc) << "Bytes" << endl;
	//cout << "Sizeof gpu_n_proc: " << sizeof(gpu_n_proc) << "Bytes" << endl;
	//cout << "Sizeof gpu_pcs: " << sizeof(gpu_pcs)/1024 << "KiB" << endl;
	//cout << "Sizeof gpu_selected: " << sizeof(gpu_selected) << "Bytes" << endl;
	//cout << "Sizeof gpu_survivals: " << sizeof(gpu_survivals)/1024 << "KiB" << endl;
	//cout << "Sizeof gpu_tournament_lengths: " << sizeof(gpu_tournament_lengths) << "Bytes" << endl;

	//  for(i = 0; i < 10; i++){
	//    cout << "Generation " << i << " max run time = " << max_tournament_length[i] << endl;
	//  }
	//  cout << "." << endl;
	//  cout << "." << endl;
	//  cout << "." << endl;
	//  for(i = MAX_GENERATIONS - 10; i < MAX_GENERATIONS; i++){
	//    cout << "Generation " << i << " max run time = " << max_tournament_length[i] << endl;
	//  }

	clFinish(commandQueue);

	// Hacky cleanup
	for(unsigned int i = 0; i<9; i++)
	{
		clReleaseMemObject(memObjects[i]);
	}
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseContext(context);
	clReleaseCommandQueue(commandQueue);

	cin.get();

	return 0;
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
					  cl_mem memObjects[8],
					  memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
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
	/*
	memObjects[1] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(pcs), pcs, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[2] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	sizeof(c_proc), c_proc, NULL);
	cout << "Cl Error: " << errNo << endl;
	memObjects[3] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	sizeof(n_proc), n_proc, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[4] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	sizeof(tournament_lengths),
	tournament_lengths, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[5] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	sizeof(survivals), survivals, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[6] = clCreateBuffer(context,
	CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	sizeof(selected), selected, &errNo);
	cout << "Cl Error: " << errNo << endl;
	*/
	memObjects[7] = clCreateBuffer(context,
		CL_MEM_WRITE_ONLY,
		sizeof(*test), NULL, &errNo);
	cout << "Cl Error: " << errNo << endl;
	memObjects[8] = clCreateBuffer(context,
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(*starts),
		starts, &errNo);
	cout << "Cl Error: " << errNo << endl;

	return true;
}

// empties everything
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

void select_programs(memory_cell pop[POPULATION_SIZE][MAX_PROGRAM_LENGTH],
					 memory_cell progs[N_PROGRAMS][MAX_PROGRAM_LENGTH],
					 int selected[N_PROGRAMS])
{
	int i,j;
	int r;

	for(i=0;i<N_PROGRAMS;i++){
		r = rand() % POPULATION_SIZE;
		selected[i] = r;
		for(j=0;j<MAX_PROGRAM_LENGTH;j++){
			progs[i][j] = pop[r][j];
		}
	}

	return;
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
			int& duration)
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
		default:
			cerr << "Bad instruction" << endl;
		}

		// update program counter
		pcs[i][c_proc[i]]++;
		pcs[i][c_proc[i]] %= MEMORY_SIZE;
		c_proc[i]++;
		if(n_proc[i] != 0) c_proc[i] %= n_proc[i]; else c_proc[i] = 0;

	}

	return;
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

/*
istream& operator>>(istream& ins, memory_cell& cell)
{
char instruct[4];
char a_mode;
int a_value;
char b_mode;
int b_value;
char dummy;

// NO ERROR CHECKING AT ALL!!!!
ins >> instruct >> a_mode >> a_value >> b_mode >> b_value;

if(!strcmp(instruct,"DAT")) cell.code = DAT;
if(!strcmp(instruct,"MOV")) cell.code = MOV;
if(!strcmp(instruct,"ADD")) cell.code = ADD;
if(!strcmp(instruct,"SUB")) cell.code = SUB;
if(!strcmp(instruct,"MUL")) cell.code = MUL;
if(!strcmp(instruct,"DIV")) cell.code = DIV;
if(!strcmp(instruct,"MOD")) cell.code = MOD;
if(!strcmp(instruct,"JMP")) cell.code = JMP;
if(!strcmp(instruct,"JMZ")) cell.code = JMZ;
if(!strcmp(instruct,"DJZ")) cell.code = DJZ;
if(!strcmp(instruct,"CMP")) cell.code = CMP;
if(!strcmp(instruct,"SPL")) cell.code = SPL;
if(!strcmp(instruct,"NOP")) cell.code = NOP;

switch(a_mode){
case '#': cell.mode_A = IMM; break;
case '$': cell.mode_A = DIR; break;
case '@': cell.mode_A = IND; break;
default: cout << "*** ERROR: undefined addressing mode ***" << endl;
}

cell.arg_A = a_value;

switch(b_mode){
case '#': cell.mode_B = IMM; break;
case '$': cell.mode_B = DIR; break;
case '@': cell.mode_B = IND; break;
default: cout << "*** ERROR: undefined addressing mode ***" << endl;
}

cell.arg_B = b_value;

return ins;
}
*/
