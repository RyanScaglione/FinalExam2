//*************************************************************************
// Ryan Scaglione
// z1996413
// CSCI463 - PE1
//
// I certify that this is my own work, and where applicable an extension
// of the starter code for the assignment
//
//*************************************************************************

/*************************************************************************
Class: registerfile

Use: Manages the general-purpose registers (x0 to x31) of the RISC-V hart.
     Provides methods to reset, set, get, and dump register values.

*************************************************************************/

#ifndef REGISTERFILE_H
#define REGISTERFILE_H

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>

/*************************************************************************
Class: registerfile

Use: Manages the general-purpose registers (x0 to x31) of the RISC-V hart.
     Provides methods to reset, set, get, and dump register values.

*************************************************************************/
class registerfile {
public:
    // Constructor

    /*************************************************************************
    Function: registerfile

    Use: Constructor initializes all registers to zero.

    Arguments: None

    ************************************************************************/
    registerfile();

    // Public Member Functions

    /*************************************************************************
    Function: reset

    Use: Resets all general-purpose registers to zero.

    Arguments: None

    ************************************************************************/
    void reset();

    /*************************************************************************
    Function: set

    Use: Sets the value of a specific register.

    Arguments:
    1. uint32_t reg: The register number (0-31).
    2. uint32_t value: The value to set.

    ************************************************************************/
    void set(uint32_t reg, uint32_t value);

    /*************************************************************************
    Function: get

    Use: Retrieves the value of a specific register.

    Arguments:
    1. uint32_t reg: The register number (0-31).

    Returns: uint32_t: The value of the register.

    ************************************************************************/
    uint32_t get(uint32_t reg) const;

    /*************************************************************************
    Function: dump

    Use: Dumps the state of the general-purpose registers.

    Arguments:
    1. const std::string &hdr: Optional header string to prefix each line.

    ************************************************************************/
    void dump(const std::string &hdr = "") const;

private:
    uint32_t regs_[32]; // General-purpose registers x0 to x31
};

#endif // REGISTERFILE_H