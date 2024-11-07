//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//Final Exam 1
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//Due date: 11/8/2024
//
//  This program simulates memory, takes an input
//
//*************************************************************************

#include "memory.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cctype>

using namespace std;

/*************************************************************************
Function: memory

Use: constructs an object of the memory class

Arguments: uint32_t s: a 32 bit integer to build the memory with

Notes: base constructor

 ************************************************************************/
memory::memory(uint32_t s)
{
    // Round up the size to closest mult of 16
    s = (s + 15) & 0xFFFFFFF0;
    mem.resize(s, 0xA5);
}

/*************************************************************************
Function: ~memory

Use: destructs the memory object

Arguments: none

Notes: base destructor

 ************************************************************************/
memory::~memory()
{
   
}

bool memory::check_illegal(uint32_t addr) const
{
    if(addr >= mem.size())
    {
        cerr << "WARNING: Address out of range: " << hex::to_hex0x32(addr) << endl;
        return true;
    }
    return false;
}

/*************************************************************************
Function: get_size()

Use: Gets the size of the memory

Arguments: none

Returns uint32_t size

Notes: lets other functions use the size 

 ************************************************************************/
uint32_t memory::get_size() const
{
    //return size of the vector
    return mem.size();
}

/*************************************************************************
Function: get8

Use: gets an 8 bit integer from the address

Arguments: 1. addr: the memory address we want to get the integer from

Returns: the byte value at addr

Notes: Does not change anything in memory

 ************************************************************************/
uint8_t memory::get8(uint32_t addr) const
{
    
    if(check_illegal(addr) == true)
    {
        return 0;
    }
    //return memory at the address
    return mem[addr];
}

/*************************************************************************
Function: get16

Use: gets a 16 bit integer from the address

Arguments: 1. addr: the memory address we want to get the integer from

Returns: the 16 bit value composed of bytes at addr and addr + 1

Notes: Does not change anything in memory

 ************************************************************************/
uint16_t memory::get16(uint32_t addr) const
{
    uint16_t lower = get8(addr);
    uint16_t upper = get8(addr + 1);
    return lower | (upper << 8);
}

/*************************************************************************
Function: get32

Use: gets an 32 bit integer from the address

Arguments: 1. addr: the memory address we want to get the integer from

Returns: the byte value at addr and addr+ 2

Notes: Does not change anything in memory

 ************************************************************************/
uint32_t memory::get32(uint32_t addr) const
{

    uint32_t lower = get16(addr);
    uint32_t upper = get16(addr + 2);
    return lower | (upper << 16);
}

/*************************************************************************
Function: get8_sx

Use: gets an 8 bit integer from the address and sign extends it to
32 bitsw

Arguments: 1. addr: the memory address we want to get the integer from

Returns: a uint32_t that is the sign extended byte

Notes: Does not change anything in memory

 ************************************************************************/
int32_t memory::get8_sx(uint32_t addr) const
{
    uint8_t val = get8(addr);
    // Sign extend to 32 
    return static_cast<int32_t>(static_cast<int8_t>(val));
}

/*************************************************************************
Function: get16_sx

Use: gets a 16 bit integer from the address and sign extends it to
32 bitsw

Arguments: 1. addr: the memory address we want to get the integer from

Returns: a uint32_t that is the sign extended byte

Notes: Does not change anything in memory

 ************************************************************************/
int32_t memory::get16_sx(uint32_t addr) const
{
    uint16_t val = get16(addr);
    // Sign extend to 32 
    return static_cast<int32_t>(static_cast<int16_t>(val));
}

/*************************************************************************
Function: get32_sx

Use: gets a 32 bit integer from the address and sign extends it to
32 bits

Arguments: 1. addr: the memory address we want to get the integer from

Returns: a uint32_t that is the sign extended byte

Notes: Does not change anything in memory

*************************************************************************/
int32_t memory::get32_sx(uint32_t addr) const
{
    uint32_t val = get32(addr);
    return static_cast<int32_t>(val);
}


/*************************************************************************
Function: set8

Use: Sets an 8 bit integer to the address

Arguments: 1. addr: the memory address we want to set the integer to
           2. val: the value we want to set

Returns: nothing

Notes: Does not work if memory is illegal
 ************************************************************************/
void memory::set8(uint32_t addr, uint8_t val)
{
    if (check_illegal(addr))
    {
        return; 
    }
    mem[addr] = val;
}

/*************************************************************************
Function: set16

Use: Calls set8 twice to account for a 16 bit value to set in memory

Arguments: 1. addr: the memory address we want to get the integer from
           2. val: the value we want to set

Returns: nothing

Notes: Inputs a 16 bit value into memory

 ************************************************************************/
void memory::set16(uint32_t addr, uint16_t val)
{
    set8(addr, static_cast<uint8_t>(val & 0xFF));
    set8(addr + 1, static_cast<uint8_t>((val >> 8) & 0xFF));
}

/*************************************************************************
Function: set32

Use: Calls set16 twice to account for a 32 bit value to set in memory

Arguments: 1. addr: the memory address we want to get the integer from
           2. val: the value we want to set

Returns: nothing

Notes: sets a 32 bit value into memory
    set8 eventually gets called from set16, which uses check_illegal

 ************************************************************************/
void memory::set32(uint32_t addr, uint32_t val)
{
    //lower bits
    set16(addr, static_cast<uint16_t>(val & 0xFFFF));
    
    //upper bits
    set16(addr + 2, static_cast<uint16_t>((val >> 16) & 0xFFFF));
}

/*************************************************************************
Function: dump

Use: Prints out the contents of the memory


Arguments: none

Returns: nothing

Notes: each line prints 16 bytes

 ************************************************************************/
void memory::dump() const
{
    const size_t line_bytes = 16;
    for(size_t i = 0; i < mem.size(); i+= line_bytes)
    {


            //address
        cout << hex::to_hex32(static_cast<uint32_t>(i)) << ": ";
        
            //bytes in hex
        for(size_t b = 0; b < line_bytes; b++)
        {
            if(i + b < mem.size())
            {
                cout << hex::to_hex8(mem[i + b]) << " ";
            }
            else
            {
                //some spaces to make the dump formatted
                cout << "   ";
            }
        }

        //some spaces to make the dump formatted
         cout << " ";



        for(size_t s = 0; s < line_bytes; s++)
        {
            if(i + s < mem.size())
            {
                uint8_t ch = get8(i + s);
                //prints non-compatible as .
                ch = isprint(ch) ? ch : '.';
                cout << static_cast<char>(ch);


            }
            else{
                //some spaces to make the dump formatted
                cout << " ";
            }
        }

        cout << endl;
    }
}

/*************************************************************************
Function: load_file

Use: loads a file into memory

Arguments: 1. fname: the npath to the file that we want to load

Returns: true if the file is loaded, false if not

Notes: Makes sure the string is 2 chars long

 ************************************************************************/
bool memory::load_file(const string &fname)
{

    ifstream infile(fname, ios::in | ios::binary);
    if(!infile)
    {
        cerr << "Can't open file " << fname << " for reading." << endl;
        return false;
    }

    //allows program to not skip white space
    infile >> noskipws;

    uint8_t byte;
    uint32_t addr = 0;
    while(infile >> byte)
    {
        if(check_illegal(addr))
        {
            cerr << "Program too big" << endl;
            infile.close();
            return false;
        }
        mem[addr++] = byte;
    }

    infile.close();
    return true;
}