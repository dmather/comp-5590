#include "sharedredcode.h"

__kernel void test(__global int *output)
{
    int gid = get_global_id(0);
    output[gid] = 5*gid;
}
