// Very simple opencl kernel
__kernel void hello_kernel(__global const float *a,
                           __global const float *b,
                           __global float *result)
{
    // get work-item id (array index)
    int gid = get_global_id(0);

    // add a and b and store them in the work-item index in result
    result[gid] = a[gid] + b[gid];
}
