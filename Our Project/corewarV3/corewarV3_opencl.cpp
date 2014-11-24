#include <iostream>
#include <stdio.h>
#include <CL/cl.h>
#include <sstream>
#include <string.h>
#include "corewarV3_opencl.h"
//#include "redcode_fileio.h"

const int ARRAY_SIZE = 1;

//using namespace std;

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

    *device = device[0];
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
        char buildLog[262144];
        errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                       sizeof(buildLog), buildLog, NULL);
        cout << "clGetProgramBuildInfo: " << errNum << endl;
        cerr << "Error in kernel: " << endl;
        cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}

bool CreateMemObjects(cl_context context, cl_mem memObjects[3],
                      float *a, float *b)
{
    memObjects[0] = clCreateBuffer(context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * ARRAY_SIZE, a, NULL);
    memObjects[1] = clCreateBuffer(context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * ARRAY_SIZE, b, NULL);
    memObjects[2] = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE,
                                   sizeof(float) * ARRAY_SIZE, NULL, NULL);
    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
    {
        printf("Error creating memory objects. %f %f %f\n", memObjects[0],
                memObjects[1], memObjects[2]);
        return false;
    }
    return true;
}

void Cleanup(cl_context context, cl_command_queue commandQueue,
             cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
    for (int i = 0; i < 3; i++)
    {
        if(memObjects[i] != 0)
            clReleaseMemObject(memObjects[i]);
    }
    if(commandQueue != 0)
        clReleaseCommandQueue(commandQueue);
    if(kernel != 0)
        clReleaseKernel(kernel);
    if(program != 0)
        clReleaseProgram(program);
    if(context != 0)
        clReleaseContext(context);
}

void init_cw(memory_cell mem[MEMORY_SIZE], int pcs[N_PROGRAMS][MAX_PROCESSES],
             int c_proc[N_PROGRAMS], int n_proc[N_PROGRAMS])
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

    for(i = 0; i < MEMORY_SIZE; i++)
    {
        mem[i] = blank;
    }

    for(i = 20; i < 25; i++)
    {
        file_one >> mem[i];
    }
    pcs[0][0] = 0;
    c_proc[0] = 0;
    n_proc[0] = 1;

    for(i = 20; i < 25; i++)
    {
        file_two >> mem[i];
    }
    pcs[1][0] = 20;
    c_proc[1] = 0;
    n_proc[1] = 1;

    file_one.close();
    file_two.close();

    return;
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

int main(int argc, char** argv)
{
    // OpenCL vars
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = {0, 0, 0};
    cl_int errNum;

    // Redcode Vars
    memory_cell memory[MEMORY_SIZE];
    int program_counters[N_PROGRAMS][MAX_PROCESSES];
    int n_processes[N_PROGRAMS];
    int cur_process[N_PROGRAMS];

    int i, j;

    // Redcode Simulator prep

    init_cw(memory, program_counters, cur_process, n_processes);

    printf("**** Programs loaded ****\n");
    for(i = 0; i < 40; i++)
    {
        cout << memory[i] << endl;
    }

    // End Redcode simulator prep

    context = CreateContext();
    if(context == NULL)
    {
        printf("Failed to create OpenCL context.\n");
        return 1;
    }

    commandQueue = CreateCommandQueue(context, &device);
    if(commandQueue == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    program = CreateProgram(context, device, "kernel.cl");
    if(program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    kernel = clCreateKernel(program, "hello_kernel", NULL);
    if(kernel == NULL)
    {
        printf("Failed to create kernel\n");
    }

    float result[ARRAY_SIZE];
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
        b[i] = i * 2;
        printf("Work-item %d a value: %f, b value: %f\n", i, a[i], b[i]);
    }

    if(!CreateMemObjects(context, memObjects, a, b))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // The first argument is the kernel object
    // The second argument is the argument index for the kernel
    // The third argument is the argument size (memory size)
    // The fourth argument is the argument value
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

    if(errNum != CL_SUCCESS)
    {
        printf("Error setting kernel arguments.\n");
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    size_t globalWorkSize[1] = { ARRAY_SIZE };
    size_t localWorkSize[1] = { 1 };

    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        printf("Error queueing kernel for execution.\n");
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0,
            ARRAY_SIZE * sizeof(float), result, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        printf("Error reading result buffer.\n");
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("%f ", result[i]);
    }

    printf("\n");
    printf("Executed program succesfully.\n");
    Cleanup(context, commandQueue, program, kernel, memObjects);

    return 0;
}
