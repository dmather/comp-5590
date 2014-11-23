// Very simple opencl kernel
typedef enum instruction {
    DAT, // not executable, values stored in B
    MOV, // copy A to B
    ADD, // add A into B (storing in B)
    SUB, // subtract A from B (storing in B)
    MUL, // multiply A into B (storing in B)
    DIV, // B / A (storing in B)
    MOD, // B mod A (storing in B)
    JMP, // transfer control to address A
    JMZ, // transfer control to address A if B is zero
    DJZ, // decrement B and if B is now 0, transfer control to A
    CMP, // if A != B skip next instruction
    SPL, // split execution into next instruction and instruction at A
    NOP  // do nothing -- used as "punctuation" for genetic algorithm
} instruction;

typedef enum addressing {
    IMM, // immediate (#)
    DIR, // direct ($)
    IND, // indirect (@)
} addressing;

struct memory_cell {
    instruction code;
    int arg_A;
    addressing mode_A;
    int arg_B;
    addressing mode_B;
};

__kernel void hello_kernel(__global const float *a,
                           __global const float *b,
                           __global float *result)
{
    // get work-item id (array index)
    int gid = get_global_id(0);

    // add a and b and store them in the work-item index in result
    result[gid] = a[gid] + b[gid];
}
