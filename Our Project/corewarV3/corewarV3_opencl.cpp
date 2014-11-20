#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <CL/cl.h>
#include "corewarV3_opencl.h"
//#include "redcode_fileio.h"

using namespace std;

int main()
{
    // To save on coding modularity some of this opencl code should be
    // plunked in a different function or maybe even file to increase
    // readability.
    cl_int error;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint platforms, devices;

    error = clGetPlatformIDs(1, &platform, &platforms);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
    }
    // For general testing I'm leaving the CL_DEVICE_TYPE_ALL in place
    // but it should be changed to CL_DEVICE_TYPE_GPU later.
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
    }
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0
    };
    cl_context context = clCreateContext(properties, 1, &device,
                                         NULL, NULL, &error);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
    }

    cl_command_queue cq = clCreateCommandQueue(context, device, 0, &error);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
    }

    // When we have our CL program uncomment this code.
    //cl_program prog = clCreateProgramWithSource(context, 1, srcptr, &srcsize,
    //                                            &error);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
    }

    // When we have our CL program uncomment this code.
    //error = clBuildProgram(prog, 0, NULL, "", NULL, NULL, NULL);
    if(error != CL_SUCCESS)
    {
        printf("Error number %d\n", error);
        //clGetProgramBuildInfo
    }

    return 0;
}
