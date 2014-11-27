#include "test.h"

__kernel void hello_kernel(__global const int *a,
                           __global const int *b,
                           __global test *result __attribute__ ((endian(host))))
{
    // get work-item id (array index)
    int gid = get_global_id(0);

    // add a and b and store them in the work-item index in result
    result[gid].opt1 = a[gid] * b[gid];
    result[gid].opt2 = 0;
    result[gid].opt3 = 0;
    result[gid].opt4 = 0;
    result[gid].opt5 = 0;
}
