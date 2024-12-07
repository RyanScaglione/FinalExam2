//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//
// 
//
//*************************************************************************

#include "rv32i_decode.h"
#include "hex.h"
#include <sstream>
#include <iomanip>

using namespace std;

/**********************************************************************
Function: get_opcode

Use: Retrieves the opcode from an instruction

Arguments:
1. insn: representation of the instruction

Returns: the opcode
**********************************************************************/
uint32_t rv32i_decode::get_opcode(uint32_t insn)
{
    return (insn & 0x7f);
}

/**********************************************************************
Function: get_rd

Use: Retrieves the destination register from an instruction

Arguments:
1. insn: the representation of the instruction

Returns: the destination register
**********************************************************************/
uint32_t rv32i_decode::get_rd(uint32_t insn)
{
     return (insn >> 7) & 0x1f;
}

/**********************************************************************
Function: get_rs1

Use: Retrieves the source register 1 from an instruction

Arguments:
1. insn: the representation of the instruction

Returns: the rs1
**********************************************************************/
uint32_t rv32i_decode::get_rs1(uint32_t insn)
{
    return (insn >> 15) & 0x1f;
}

/**********************************************************************
Function: get_rs2

Use: Retrieves the source register 2 from an instruction

Arguments:
1. insn: the representation of the instruction

Returns: the rs2
**********************************************************************/
uint32_t rv32i_decode::get_rs2(uint32_t insn)
{
    return (insn >> 20) & 0x1f;
}

/**********************************************************************
Function: get_funct3

Use: Retrieves the funct3 from an instruction

Arguments:
1. insn: the representation of the instruction

Returns: the funct3
**********************************************************************/
uint32_t rv32i_decode::get_funct3(uint32_t insn)
{
    return (insn >> 12) & 0x7;
}

/**********************************************************************
Function: get_funct7

Use: Retrieves the funct7 from an instruction

Arguments:
1. insn: the representation of the instruction

Returns: the funct7
**********************************************************************/
uint32_t rv32i_decode::get_funct7(uint32_t insn)
{
    return (insn >> 25) & 0x7f;
}

/**********************************************************************
Function: get_imm_i

Use: Retrieves the immediate value for I instructions

Arguments:
1. insn: the representation of the instruction

Returns: the immediate value as an int
**********************************************************************/
int32_t rv32i_decode::get_imm_i(uint32_t insn)
{
    int32_t imm = (insn >> 20);
    if (insn & 0x80000000)
    {
        imm |= 0xfffff000; // sign extend tro 32
    }
        
    return imm;
}

/**********************************************************************
Function: get_imm_u

Use: Retrieves the immediate value for U instructions 

Arguments:
1. insn: the representation of the instruction

Returns: the immediate value as an int
**********************************************************************/
int32_t rv32i_decode::get_imm_u(uint32_t insn)
{
    
    return (insn >> 12) & 0xfffff;
}

/**********************************************************************
Function: get_imm_b

Use: Retrieves the immediate value for B instructions

Arguments:
1. insn: The representation of the instruction

Returns: the immediate value as an int
**********************************************************************/
int32_t rv32i_decode::get_imm_b(uint32_t insn)
{
    int32_t imm = 0;

    imm |= ((insn >> (31 - 12)) & (1 << 12));   
    imm |= ((insn >> (25 - 5)) & 0x7e0);        
    imm |= ((insn >> (8 - 1)) & 0x1e);          
    imm |= ((insn << (11 - 7)) & (1 << 11));    

    if (imm & (1 << 12))
    {
         imm |= 0xffffe000;
    }
       

    return imm;
}

/**********************************************************************
Function: get_imm_s

Use: Retrieves the immediate value for S instructions

Arguments:
1. insn: The representation of the instruction

Returns: the immediate value as an int
**********************************************************************/
int32_t rv32i_decode::get_imm_s(uint32_t insn)
{
    int32_t imm_s = (insn & 0xfe000000) >> (25 - 5);
    imm_s |= (insn & 0x00000f80) >> (7 - 0);
    if (insn & 0x80000000)
    {
        imm_s |= 0xfffff000; 
    }
        
    return imm_s;
}

/**********************************************************************
Function: get_imm_j

Use: Retrieves the immediate value for J instructions

Arguments:
1. insn: The representation of the instruction

Returns: the immediate value as an int
**********************************************************************/
int32_t rv32i_decode::get_imm_j(uint32_t insn)
{
    int32_t imm = 0;

    imm |= (insn & 0xff000);                     
    imm |= ((insn & (1 << 20)) >> 9);             
    imm |= ((insn >> (21 - 1)) & 0x7fe);          
    imm |= ((insn >> (31 - 20)) & (1 << 20));     

    // Sign-extend
    if (imm & (1 << 20))
    {
        imm |= 0xfff00000;
    }
        

    return imm;
}

/**********************************************************************
Function: decode

Use: Decodes the instruction and returns the representation

Arguments:
1. addr: The address of the instruction
2. insn: The representation of the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::decode(uint32_t addr, uint32_t insn)
{
    uint32_t opcode = get_opcode(insn);
    switch (opcode)
    {
        case opcode_lui:
        {

        
            return render_lui(insn);
        }
        case opcode_auipc:
        {
            return render_auipc(insn);
        }
        case opcode_jal:
        {
            return render_jal(addr, insn);
        }
        case opcode_jalr:
        {
            return render_jalr(insn);
        }
        case opcode_branch:
        {
            uint32_t funct3 = get_funct3(insn);
            switch (funct3)
            {
            case funct3_beq:
            {
                return render_btype(addr, insn, "beq");
            }
                
            case funct3_bne:
            {
                return render_btype(addr, insn, "bne");
            }
                
            case funct3_blt:
            {
                return render_btype(addr, insn, "blt");
            }
            
            case funct3_bge:
            {
                return render_btype(addr, insn, "bge");
            }
                
            case funct3_bltu:
            {
                return render_btype(addr, insn, "bltu");
            }
                
            case funct3_bgeu:
            {
                return render_btype(addr, insn, "bgeu");
            }
                
            default:
            {
                return render_illegal_insn();
            }
                
            }
        }
        case opcode_load:
        {
            uint32_t funct3 = get_funct3(insn);
            switch (funct3)
            {
            case funct3_lb:
            {
                return render_itype_load(insn, "lb");
            }
                
            case funct3_lh:
            {
                return render_itype_load(insn, "lh");
            }
                
            case funct3_lw:
            {
                return render_itype_load(insn, "lw");
            }
                
            case funct3_lbu:
            {
                return render_itype_load(insn, "lbu");
            }
                
            case funct3_lhu:
            {
                return render_itype_load(insn, "lhu");
            }
                
            default:
            {
                return render_illegal_insn();
            }
                
            }
        }
        case opcode_store:
        {
            uint32_t funct3 = get_funct3(insn);
            switch (funct3)
            {
                case funct3_sb:
                {
                    return render_stype(insn, "sb");
                }
            
                case funct3_sh:
                {
                    return render_stype(insn, "sh");
                }
                
                case funct3_sw:
                {
                    return render_stype(insn, "sw");
                }
                    
                default:
                {
                    return render_illegal_insn();
                }
                    
            }
        }
        case opcode_alu_imm:
        {
            uint32_t funct3 = get_funct3(insn);
            int32_t imm_i = get_imm_i(insn);
            switch (funct3)
            {
                case funct3_addi:
                {
                    return render_itype_alu(insn, "addi", imm_i);
                }
                case funct3_slti:
                {
                    return render_itype_alu(insn, "slti", imm_i);
                }
                case funct3_sltiu:
                {
                    return render_itype_alu(insn, "sltiu", imm_i);
                }
                
                case funct3_xori:
                {
                    return render_itype_alu(insn, "xori", imm_i);
                }
                
                case funct3_ori:
                {
                    return render_itype_alu(insn, "ori", imm_i);
                }
                
                case funct3_andi:
                {
                    return render_itype_alu(insn, "andi", imm_i);
                }
                
                case funct3_slli:
                {
                    if (get_funct7(insn) == 0b0000000)
                    {
                        uint32_t shamt = insn >> 20 & 0x1f;
                        return render_itype_alu(insn, "slli", shamt);
                    }
                    else
                    { 
                        return render_illegal_insn();
                    }
                }
                case funct3_srli_srai:
                {
                    uint32_t shamt = insn >> 20 & 0x1f;
                    uint32_t funct7 = get_funct7(insn);
                    if (funct7 == 0b0000000)
                    {
                        return render_itype_alu(insn, "srli", shamt);
                    }
                    else if (funct7 == 0b0100000)
                    {
                        return render_itype_alu(insn, "srai", shamt);
                    }
                    else
                    {
                        return render_illegal_insn();
                    }
                    
                }
                default:
                {
                    return render_illegal_insn();
                }
            }
        }
        case opcode_alu_reg:
        {
            uint32_t funct3 = get_funct3(insn);
            uint32_t funct7 = get_funct7(insn);
            switch (funct3)
            {
                case funct3_add_sub:
                {
                    if (funct7 == funct7_add)
                    {
                        return render_rtype(insn, "add");
                    }
                        
                    else if (funct7 == funct7_sub)
                    {
                        return render_rtype(insn, "sub");
                    }
                        
                    else
                    {
                        return render_illegal_insn();
                    }      
                }
                    
                case funct3_sll:
                {
                    if (funct7 == funct7_sll)
                    {
                        return render_rtype(insn, "sll");
                    }
                        
                    else
                    {
                        return render_illegal_insn();
                    }
                }
                
                case funct3_slt:
                {
                    if (funct7 == funct7_slt)
                    {
                        return render_rtype(insn, "slt");
                    }                 
                    else
                    {
                        return render_illegal_insn();
                    } 
                }                
                case funct3_sltu:
                {
                    if (funct7 == funct7_sltu)
                    {
                        return render_rtype(insn, "sltu");
                    }                  
                    else
                    {
                        return render_illegal_insn();
                    }
                }
                    
                case funct3_xor:
                {
                    if (funct7 == funct7_xor)
                    {
                        return render_rtype(insn, "xor");
                    } 
                    else
                    {
                        return render_illegal_insn();
                    }
                }
                    
                case funct3_srl_sra:
                {
                    if (funct7 == funct7_srl)
                    {
                        return render_rtype(insn, "srl");
                    }
                        
                    else if (funct7 == funct7_sra)
                    {
                        return render_rtype(insn, "sra");
                    }
                        
                    else
                    {
                        return render_illegal_insn();
                    }        
                }

                case funct3_or:
                {
                    if (funct7 == funct7_or)
                    {
                        return render_rtype(insn, "or");
                    }
                    else
                    {
                        return render_illegal_insn();
                    } 
                }
                    
                case funct3_and:
                {
                    if (funct7 == funct7_and)
                    {
                        return render_rtype(insn, "and");
                    }   
                    else
                    {
                        return render_illegal_insn();
                    }
                }
                    
                default:
                {
                    return render_illegal_insn();
                }
            }
        }
        

        case opcode_system:
        {
            uint32_t funct3 = get_funct3(insn);
    
            if (insn == 0x00000073)
            {
                return render_mnemonic("ecall");
            }
            else if (insn == 0x00100073)
            {
                return render_mnemonic("ebreak");
            }
            else if (funct3 == funct3_csrrw)
            {
                return render_csrrx(insn, "csrrw");

            }
            else if (funct3 == funct3_csrrs)
            {
                return render_csrrx(insn, "csrrs");
            }
            else if (funct3 == funct3_csrrc)
            {
                return render_csrrx(insn, "csrrc");
            }
            else if (funct3 == funct3_csrrwi)
            {
                return render_csrrxi(insn, "csrrwi");
            }
            else if (funct3 == funct3_csrrsi)
            {
                return render_csrrxi(insn, "csrrsi");
            }
            else if (funct3 == funct3_csrrci)
            {
                return render_csrrxi(insn, "csrrci");
            }
                return render_illegal_insn();
        }
        default:
        {
            return render_illegal_insn();
        }
        
    }
}



/**********************************************************************
Function: render_illegal_insn

Use: Renders an illegal instruction message

Arguments: None

Returns: An error message 
**********************************************************************/
string rv32i_decode::render_illegal_insn()
{
    return "ERROR: UNIMPLEMENTED INSTRUCTION";
}

/**********************************************************************
Function: render_lui

Use: Renders the LUI instruction

Arguments:
1. insn: The representation of the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_lui(uint32_t insn)
{
    int rd = get_rd(insn);
    int32_t imm_u = get_imm_u(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic("lui") << render_reg(rd) << ","
    << hex::to_hex0x20(imm_u);
    return stringstream.str();
}

/**********************************************************************
Function: render_auipc

Use: Renders the AUIPC instruction

Arguments:
1. insn: The representation of the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_auipc(uint32_t insn)
{
    int rd = get_rd(insn);
    int32_t imm_u = get_imm_u(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic("auipc") << render_reg(rd) << ","
    << hex::to_hex0x20(imm_u);
    return stringstream.str();
}

/**********************************************************************
Function: render_jal

Use: Renders the JAL instruction

Arguments:
1. addr: The address of the instruction
2. insn: The representation of the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_jal(uint32_t addr, uint32_t insn)
{
    int rd = get_rd(insn);
    int32_t imm_j = get_imm_j(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic("jal") << render_reg(rd) << ", "
    << hex::to_hex0x32(addr + imm_j);
    return stringstream.str();
}

/**********************************************************************
Function: render_jalr

Use: Renders the JALR instruction

Arguments:
1. insn: The representation of the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_jalr(uint32_t insn)
{
    int rd = get_rd(insn);
    int rs1 = get_rs1(insn);
    int32_t imm_i = get_imm_i(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic("jalr") << render_reg(rd) << ", "
    << render_base_disp(imm_i, rs1);
    return stringstream.str();
}

/**********************************************************************
Function: render_btype

Use: Renders a B instruction

Arguments:
1. addr: The address of the instruction
2. insn: The representation of the instruction
3. mnemonic: The mnemonic string for the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_btype(uint32_t addr, uint32_t insn, const string &mnemonic)
{
    int rs1 = get_rs1(insn);
    int rs2 = get_rs2(insn);
    int32_t imm_b = get_imm_b(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic(mnemonic)<< render_reg(rs1) << ", "
    << render_reg(rs2) << ", " << hex::to_hex0x32(addr + imm_b);
    return stringstream.str();
}

/**********************************************************************
Function: render_itype_load

Use: Renders an I load instruction

Arguments:
1. insn: The representation of the instruction
2. mnemonic: The mnemonic string for the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_itype_load(uint32_t insn, const string &mnemonic)
{
    int rd = get_rd(insn);
    int rs1 = get_rs1(insn);
    int32_t imm_i = get_imm_i(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic(mnemonic) << render_reg(rd) << ", "
    << render_base_disp(imm_i, rs1);
    return stringstream.str();
}

/**********************************************************************
Function: render_stype

Use: Renders an S instruction

Arguments:
1. insn: The representation of the instruction
2. mnemonic: The mnemonic string for the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_stype(uint32_t insn, const string &mnemonic)
{
    int rs1 = get_rs1(insn);
    int rs2 = get_rs2(insn);
    int32_t imm_s = get_imm_s(insn);
    ostringstream stringstream;

    
    stringstream << render_mnemonic(mnemonic) << render_reg(rs2) << ", "
    << render_base_disp(imm_s, rs1);
    return stringstream.str();
}

/**********************************************************************
Function: render_itype_alu

Use: Renders an IALU instruction

Arguments:
1. insn: The representation of the instruction
2. mnemonic: The mnemonic string for the instruction
3. imm_i: The immediate value

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_itype_alu(uint32_t insn, const string &mnemonic, int32_t imm_i)
{
    int rd = get_rd(insn);
    int rs1 = get_rs1(insn);
    ostringstream stringstream;

   
    stringstream << render_mnemonic(mnemonic) << render_reg(rd) << ", " 
    << render_reg(rs1) << ", " << imm_i;
    return stringstream.str();
}

/**********************************************************************
Function: render_rtype

Use: Renders an R ALU instruction

Arguments:
1. insn: The representation of the instruction
2. mnemonic: The mnemonic string for the instruction

Returns: A string with the disassembled instruction
**********************************************************************/
string rv32i_decode::render_rtype(uint32_t insn, const string &mnemonic)
{
    int rd = get_rd(insn);
    int rs1 = get_rs1(insn);
    int rs2 = get_rs2(insn);
    ostringstream stringstream;

    stringstream << render_mnemonic(mnemonic)  << render_reg(rd) << ", "   
    << render_reg(rs1) << ", " << render_reg(rs2);
    return stringstream.str();
}

/**********************************************************************
Function: render_reg

Use: Renders a register name

Arguments:
1. r: The register number

Returns: A string with the register name
**********************************************************************/
string rv32i_decode::render_reg(int r)
{
    ostringstream stringstream;
    stringstream << "x" << r;
    return stringstream.str();
}

/**********************************************************************
Function: render_base_disp

Use: Renders a base and displacement addressing

Arguments:1. imm: The immediate value
2. r: The register number

Returns: A string with the base and displacement
**********************************************************************/
string rv32i_decode::render_base_disp(int32_t imm, int r)
{
    ostringstream stringstream;
    stringstream << imm << "(" << render_reg(r) << ")";
    return stringstream.str();
}

/**********************************************************************
Function: render_mnemonic

Use: Renders a mnemonic

Arguments: 1. &mnemonic: reference to the mnemonic string 

Returns: A formatted string with the mnemonic
**********************************************************************/
string rv32i_decode::render_mnemonic(const string &mnemonic)
{
    if (mnemonic == "ecall" || mnemonic == "ebreak")
    {
        return mnemonic;
    }
        


    ostringstream stringstream;
    stringstream << left << setw(mnemonic_width) << mnemonic;
    return stringstream.str();
}

/**********************************************************************
Function: render_csrrx

Use: Renders a CSR instruction

Arguments: 1:insn, the representation of the instruction
2: &mnemonic: reference to the string to render


Returns: A formatted string with the mnemonic
**********************************************************************/
string rv32i_decode::render_csrrx(uint32_t insn, const string &mnemonic)
{
    uint32_t rd = get_rd(insn);
    uint32_t rs1 = get_rs1(insn);
    uint32_t csr = (insn >> 20) & 0xfff;
    ostringstream stringstream;
 
   
    stringstream << render_mnemonic(mnemonic) << render_reg(rd) << "," << 
    hex::to_hex0x12(csr) << "," << render_reg(rs1);
    return stringstream.str();
}

/**********************************************************************
Function: render_csrrxi

Use: Renders a CSR instruction

Arguments: 1:insn, the representation of the instruction
2: &mnemonic: reference to the string to render


Returns: A formatted string with the mnemonic
**********************************************************************/
string rv32i_decode::render_csrrxi(uint32_t insn, const string &mnemonic)
{
    uint32_t rd = get_rd(insn);
    uint32_t immz = get_rs1(insn);
    uint32_t csr = (insn >> 20) & 0xfff;
    ostringstream stringstream;

    stringstream << render_mnemonic(mnemonic) << render_reg(rd) << ","
    << hex::to_hex0x12(csr) << "," << immz;
    return stringstream.str();
}