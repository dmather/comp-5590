#ifndef SHAREDREDCODE_H
#define SHAREDREDCODE_H

#define MEMORY_SIZE 40 // just little for debugging
#define N_PROGRAMS 2 // first just test each instruction
#define MAX_PROCESSES 1000
#define MAX_ITERATIONS 10

enum instruction {
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
};

enum addressing {
    IMM, // immediate (#)
    DIR, // direct ($)
    IND, // indirect (@)
};

struct memory_cell {
    instruction code;
    int arg_A;
    addressing mode_A;
    int arg_B;
    addressing mode_B;
};

#endif // SHAREDREDCODE_H
