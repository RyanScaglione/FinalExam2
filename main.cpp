//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//
//*************************************************************************


#include <iomanip>
#include <iostream>
#include <sstream>    
#include <cstdlib>    
#include <unistd.h>
#include <string>   
#include "memory.h"   
#include "hex.h"  
#include "rv32i_decode.h"


using namespace std;

static void usage();

static void usage()
{
	cerr << "Usage: rv32i [-m hex-mem-size] infile" << endl;
	cerr << "    -m specify memory size (default = 0x100)" << endl;
	exit(1);
}


/*************************************************************************
Function: disassemble

Use: iterates through memory and decodes every 32-bit instruction 

Arguments: 1. &mem: reference to the memory


Returns: void
 ************************************************************************/
static void disassemble(const memory &mem)
{
    for (uint32_t addr = 0; addr < mem.get_size(); addr += 4)
    {
        uint32_t insn = mem.get32(addr);
        string decoded = rv32i_decode::decode(addr, insn);
        cout << hex::to_hex32(addr) << ": " << hex::to_hex32(insn) << "  " << decoded << endl;
    }
}
/*************************************************************************
Function: main

Use: The main driver function 

Arguments: 1. argc: the numbver of arguments
		   2. argv: array of arguments

Returns: 0 if it's successful
 ************************************************************************/
int main(int argc, char **argv)
{
	uint32_t memory_limit = 0x100; // default memory size = 256 bytes
	int opt;
	while ((opt = getopt(argc, argv, "m:")) != -1)
	{
		switch (opt)
		{
			case 'm':
			{
				std::istringstream iss(optarg);
				iss >> std::hex >> memory_limit;
			}
			break;
		default: /* ’?’ */
			usage();
		}
	}
	if (optind >= argc)
		usage(); // missing filename

	memory mem(memory_limit);

	if (!mem.load_file(argv[optind]))
		usage();


	disassemble(mem);
	mem.dump();

	return 0;
}