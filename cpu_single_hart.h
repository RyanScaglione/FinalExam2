// cpu_single_hart.h
//*************************************************************************
// Ryan Scaglione
// z1996413
// CSCI463 - PE1
//
// I certify that this is my own work, and where applicable an extension
// of the starter code for the assignment
//
//*************************************************************************

#ifndef CPU_SINGLE_HART_H
#define CPU_SINGLE_HART_H

#include "rv32i_hart.h"
#include "memory.h"

class cpu_single_hart
{
public:
    // Constructor
    cpu_single_hart(memory &m, uint32_t exec_limit = 0);

    // Getter for the hart
    rv32i_hart& get_hart();

    // Run the simulation
    void run();

    // Setters for simulation parameters
    void set_exec_limit(uint32_t limit);
    void set_show_instructions(bool b);
    void set_show_registers(bool b);

private:
    rv32i_hart hart;            // Single hart instance
    uint32_t execution_limit;   
};

#endif // CPU_SINGLE_HART_H
