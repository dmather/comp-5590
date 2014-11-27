__kernel void hello_kernel(__global const int *a,
                           __global const int *b,
                           __global int *result)
{
    // get work-item id (array index)
    int gid = get_global_id(0);

    // add a and b and store them in the work-item index in result
    result[gid] = a[gid] + b[gid];
}
