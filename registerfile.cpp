//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//
//*************************************************************************

/*************************************************************************
registerfile.cpp

Implementation of the registerfile class, which manages the general-purpose registers
of the RISC-V hart.

*************************************************************************/

#include "registerfile.h"

/*************************************************************************
Function: registerfile

Use: Constructor initializes all registers to zero.

Arguments: None

 ************************************************************************/
registerfile::registerfile()
{
    reset();
}

/*************************************************************************
Function: reset

Use: Resets all general-purpose registers to zero.

Arguments: None

 ************************************************************************/
void registerfile::reset()
{
    for(int i = 0; i < 32; ++i) {
        regs_[i] = 0;
    }
}

/*************************************************************************
Function: set

Use: Sets the value of a specific register.

Arguments:
1. uint32_t reg: The register number (0-31).
2. uint32_t value: The value to set.

 ************************************************************************/
void registerfile::set(uint32_t reg, uint32_t value)
{
    if(reg == 0) {
        // x0 is always zero; cannot be modified
        return;
    }
    if(reg < 32) {
        regs_[reg] = value;
    }
    else {
        // Invalid register number; handle as needed (e.g., ignore or log error)
    }
}

/*************************************************************************
Function: get

Use: Retrieves the value of a specific register.

Arguments:
1. uint32_t reg: The register number (0-31).

Returns: uint32_t: The value of the register.

 ************************************************************************/
uint32_t registerfile::get(uint32_t reg) const
{
    if(reg < 32) {
        return regs_[reg];
    }
    else {
        // Invalid register number; return 0 or handle as needed
        return 0;
    }
}

/*************************************************************************
Function: dump

Use: Dumps the state of the general-purpose registers.

Arguments:
1. const std::string &hdr: Optional header string to prefix each line.

 ************************************************************************/
void registerfile::dump(const std::string &hdr) const
{
    for(int i = 0; i < 32; i +=8 ) {
        std::cout << hdr << "x" << i;
        for(int j = 0; j < 8; ++j) {
            std::cout << " " << std::hex << std::setw(8) << std::setfill('0') << regs_[i+j];
        }
        std::cout << std::dec << std::endl;
    }
}