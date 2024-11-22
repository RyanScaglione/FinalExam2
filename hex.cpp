//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//
//*************************************************************************


#include <iostream>
#include <sstream>
#include "hex.h"
#include <iomanip>

using namespace std;

/*************************************************************************
Function: to_hex8

Use: convert an 8 bit integer to a 2 digit hex representation

Arguments: 1. uint8_t i: the integer we want to convert

Notes: Makes sure the string is 2 chars long

 ************************************************************************/
string hex::to_hex8(uint8_t i)
{
    //
    ostringstream os;
    //have to use std:: since it'll grab the hex class instead
    os << std::hex << setfill('0') << setw(2) << static_cast<uint16_t>(i);
    return os.str();

} 

/*************************************************************************
Function: to_hex32

Use: convert an 32 bit integer to an 8 digit hex representation

Arguments: 1. uint32_t i: the integer we want to convert

Notes: Makes sure the string is 8 char long

 ************************************************************************/
string hex::to_hex32(uint32_t i)
{
    //
    ostringstream os;
    os << std::hex << setfill('0') << setw(8) << i;
    return os.str();

} 

/*************************************************************************
Function: to_hex32

Use: convert an 32 bit integer to an 8 digit hex representation, with a
0x used at the start

Arguments: 1. uint32_t i: the integer we want to convert

Notes: Makes sure the string is 8 char long

 ************************************************************************/
string hex::to_hex0x32(uint32_t i)
{
    return string("0x") + to_hex32(i);
}

/*************************************************************************
Function: to_hex0x20

Use: Converts to a hex string with 20 LSB's

Arguments: 1. uint32_t i: the integer we want to convert


 ************************************************************************/
string hex::to_hex0x20(uint32_t i)
{
    std::ostringstream oss;
    oss << "0x" << setfill('0') << setw(5) << std::hex << (i & 0xfffff);
    return oss.str();
}

/*************************************************************************
Function: to_hex0x12

Use: Converts to a hex string with 12 LSB's

Arguments: 1. uint32_t i: the integer we want to convert


 ************************************************************************/
string hex::to_hex0x12(uint32_t i)
{
    ostringstream oss;
    oss << "0x" << std::hex << setw(3) << setfill('0') << (i & 0x00000fff);
    return oss.str();
}

