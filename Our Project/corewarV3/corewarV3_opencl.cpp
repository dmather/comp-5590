#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <CL/cl.h>
#include <fstream>
#include <sstream>
#include "corewarV3_opencl.h"
//#include "redcode_fileio.h"

const int ARRAY_SIZE = 100;

using namespace std;

cl_context CreateContext(void)
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;

    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
    if(errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        printf("Failed to find any OpenCL platforms.");
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
        printf("could not create GPU context, trying CPU...");
        context = clCreateContextFromType(contextProperties,
                                          CL_DEVICE_TYPE_CPU, NULL, NULL,
                                          &errNum);
        if(errNum != CL_SUCCESS)
        {
            printf("Failed to create an OpenCL GPU or CPU context.");
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
        printf("Failed called to clGetContextInf(..., CL_CONTEXT_DEVICES, ...)");
        return NULL;
    }

    if(deviceBufferSize <= 0)
    {
        printf("No devices available.");
        return NULL;
    }

    devices = new cl_device_id[deviceBufferSize/sizeof(cl_device_id)];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize,
                              devices, NULL);
    if(errNum != CL_SUCCESS)
    {
        printf("Failed to get device IDs");
        delete[] devices;
        return NULL;
    }

    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if(commandQueue == NULL)
    {
        printf("Failed to create command Queue for device 0");
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
        printf("Failed to open file for reading: %s", fileName);
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

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);
        printf("Error in kernel: ");
        printf("%s", buildLog);
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}

bool CreateMemObjects(cl_context context, cl_mem memObjects[3],
                      int *a, int *b)
{
    memObjects[0] = clCreateBuffer(context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int) * ARRAY_SIZE, a, NULL);
    memObjects[1] = clCreateBuffer(context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int) * ARRAY_SIZE, b, NULL);
    memObjects[2] = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE,
                                   sizeof(int) * ARRAY_SIZE, NULL, NULL);
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

int main(int argc, char** argv)
{
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = {0, 0, 0};
    cl_int errNum;

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

    int result[ARRAY_SIZE];
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
        b[i] = i * 2;
    }

    if(!CreateMemObjects(context, memObjects, a, b))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

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
            ARRAY_SIZE * sizeof(int), result, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        printf("Error reading result buffer.\n");
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("%d ", result[i]);
    }

    printf("\n");
    printf("Executed program succesfully.\n");
    Cleanup(context, commandQueue, program, kernel, memObjects);

    return 0;
}
