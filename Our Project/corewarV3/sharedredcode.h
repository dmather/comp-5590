#ifndef SHAREDREDCODE_H
#define SHAREDREDCODE_H

#define MEMORY_SIZE 500 // just little for debugging
#define N_PROGRAMS 4 // first just test each instruction
#define MAX_PROCESSES 100
#define MAX_STEPS 1000
#define MAX_PROGRAM_LENGTH 100
#define POPULATION_SIZE 40
#define MAX_GENERATIONS 20
#define N_TOURNAMENTS 600
#define N_MODES 3
#define N_OPERATIONS 13

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

typedef struct memory_cell {
    instruction code;
    int arg_A;
    addressing mode_A;
    int arg_B;
    addressing mode_B;
} memory_cell;

#endif // SHAREDREDCODE_H
