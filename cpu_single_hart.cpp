//*************************************************************************
// Ryan Scaglione
// z1996413
// CSCI463 - PE1
//
// I certify that this is my own work, and where applicable an extension
// of the starter code for the assignment
//
//*************************************************************************

#include "rv32i_hart.h"
#include "rv32i_decode.h"
#include "registerfile.h"
#include "memory.h"
#include "hex.h"
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <sstream>
#include <stdexcept>


rv32i_hart::rv32i_hart()
    : pc(0), halt(false), halt_reason(""), decoder(), regs(), mem()
{
    // Initialize CSR map with standard CSRs (modify as needed)
    csr_map[0x300] = 0; // mhartid
    csr_map[0x301] = 0; // mstatus
    csr_map[0x342] = 0; // mtvec
    // Add other CSRs as necessary

    // Initialize specific registers if needed
    // Example: Initialize x6 to 100 as per your test program
    regs.set(6, 100);
}

// The main execution function 
void rv32i_hart::exec(uint32_t insn, std::ostream* pos)
{
    uint32_t opcode = decoder.get_opcode(insn);

    switch (opcode)
    {
        default:
            exec_illegal_insn(insn, pos);
            return;

        // U-Type Instructions
        case rv32i_decode::opcode_lui:
            exec_lui(insn, pos);
            return;

        case rv32i_decode::opcode_auipc:
            exec_auipc(insn, pos);
            return;

        // J-Type Instruction
        case rv32i_decode::opcode_jal:
            exec_jal(insn, pos);
            return;

        // I-Type Instructions
        case rv32i_decode::opcode_jalr:
            exec_jalr(insn, pos);
            return;

        case rv32i_decode::opcode_load:
            {
                uint32_t funct3 = decoder.get_funct3(insn);
                switch(funct3)
                {
                    case rv32i_decode::funct3_lb:
                        exec_lb(insn, pos);
                        break;
                    case rv32i_decode::funct3_lh:
                        exec_lh(insn, pos);
                        break;
                    case rv32i_decode::funct3_lw:
                        exec_lw(insn, pos);
                        break;
                    case rv32i_decode::funct3_lbu:
                        exec_lbu(insn, pos);
                        break;
                    case rv32i_decode::funct3_lhu:
                        exec_lhu(insn, pos);
                        break;
                    default:
                        exec_illegal_insn(insn, pos);
                        break;
                }
            }
            return;

        case rv32i_decode::opcode_store:
            {
                uint32_t funct3 = decoder.get_funct3(insn);
                switch(funct3)
                {
                    case rv32i_decode::funct3_sb:
                        exec_sb(insn, pos);
                        break;
                    case rv32i_decode::funct3_sh:
                        exec_sh(insn, pos);
                        break;
                    case rv32i_decode::funct3_sw:
                        exec_sw(insn, pos);
                        break;
                    default:
                        exec_illegal_insn(insn, pos);
                        break;
                }
            }
            return;

        case rv32i_decode::opcode_alu_imm:
            {
                uint32_t funct3 = decoder.get_funct3(insn);
                switch(funct3)
                {
                    case rv32i_decode::funct3_addi:
                        exec_addi(insn, pos);
                        break;
                    case rv32i_decode::funct3_slti:
                        exec_slti(insn, pos);
                        break;
                    case rv32i_decode::funct3_sltiu:
                        exec_sltiu(insn, pos);
                        break;
                    case rv32i_decode::funct3_xori:
                        exec_xori(insn, pos);
                        break;
                    case rv32i_decode::funct3_ori:
                        exec_ori(insn, pos);
                        break;
                    case rv32i_decode::funct3_andi:
                        exec_andi(insn, pos);
                        break;
                    case rv32i_decode::funct3_slli:
                        exec_slli(insn, pos);
                        break;
                    case rv32i_decode::funct3_srli_srai:
                        exec_srli_srai(insn, pos);
                        break;
                    default:
                        exec_illegal_insn(insn, pos);
                        break;
                }
            }
            return;

        case rv32i_decode::opcode_alu_reg:
            {
                uint32_t funct3 = decoder.get_funct3(insn);
                uint32_t funct7 = decoder.get_funct7(insn);
                switch(funct3)
                {
                    case rv32i_decode::funct3_add_sub:
                        if(funct7 == rv32i_decode::funct7_add)
                            exec_add(insn, pos);
                        else if(funct7 == rv32i_decode::funct7_sub)
                            exec_sub(insn, pos);
                        else
                            exec_illegal_insn(insn, pos);
                        break;
                    case rv32i_decode::funct3_sll:
                        if(funct7 == rv32i_decode::funct7_sll)
                            exec_sll(insn, pos);
                        else
                            exec_illegal_insn(insn, pos);
                        break;
                    case rv32i_decode::funct3_slt:
                        exec_slt(insn, pos);
                        break;
                    case rv32i_decode::funct3_sltu:
                        exec_sltu(insn, pos);
                        break;
                    case rv32i_decode::funct3_xor:
                        exec_xor(insn, pos);
                        break;
                    case rv32i_decode::funct3_srl_sra:
                        if(funct7 == rv32i_decode::funct7_srl)
                            exec_srl(insn, pos);
                        else if(funct7 == rv32i_decode::funct7_sra)
                            exec_sra(insn, pos);
                        else
                            exec_illegal_insn(insn, pos);
                        break;
                    case rv32i_decode::funct3_or:
                        exec_or(insn, pos);
                        break;
                    case rv32i_decode::funct3_and:
                        exec_and(insn, pos);
                        break;
                    default:
                        exec_illegal_insn(insn, pos);
                        break;
                }
            }
            return;

        case rv32i_decode::opcode_system:
            exec_system(insn, pos);
            return;
    }
}

/**********************************************************************
Function: exec_lui

Use: Executes the LUI (Load Upper Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lui(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    int32_t imm_u = decoder.get_imm_u(insn); // Immediate is upper 20 bits

    // Set rd to immediate value
    regs.set(rd, imm_u);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_lui(insn);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = " << hex::to_hex0x32(imm_u) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_auipc

Use: Executes the AUIPC (Add Upper Immediate to PC) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_auipc(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    int32_t imm_u = decoder.get_imm_u(insn); // Immediate is upper 20 bits

    // Calculate PC + immediate
    uint32_t result = pc + imm_u;

    // Set rd to result
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_auipc(insn);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = PC (" << hex::to_hex32(pc) << ") + " 
             << hex::to_hex0x32(imm_u) << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_jal

Use: Executes the JAL (Jump and Link) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_jal(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    int32_t imm_j = decoder.get_imm_j(insn); // Immediate for JAL

    // Save PC + 4 to rd
    regs.set(rd, pc + 4);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_jal(pc, insn);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = PC + 4 (" << hex::to_hex32(pc + 4) 
             << "), PC += " << hex::to_hex0x32(imm_j) << std::endl;
    }

    // Jump to PC + immediate
    pc += imm_j;
}

/**********************************************************************
Function: exec_jalr

Use: Executes the JALR (Jump and Link Register) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_jalr(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate target address: (rs1 + imm) & ~1
    uint32_t target = (regs.get(rs1) + imm_i) & ~1;

    // Save PC + 4 to rd
    regs.set(rd, pc + 4);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_jalr(insn);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = PC + 4 (" << hex::to_hex32(pc + 4) 
             << "), PC = " << hex::to_hex32(target) << std::endl;
    }

    // Jump to target address
    pc = target;
}

/**********************************************************************
Function: exec_lb

Use: Executes the LB (Load Byte) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lb(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    try {
        // Load byte from memory
        int8_t loaded_byte = mem.get8(addr);

        // Sign-extend to 32 bits
        int32_t value = static_cast<int32_t>(loaded_byte);

        // Set rd
        regs.set(rd, static_cast<uint32_t>(value));

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_load(insn, "lb");
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = sign_extend(mem[" << hex::to_hex32(addr) 
                 << "] = " << hex::to_hex0x32(static_cast<uint8_t>(loaded_byte)) 
                 << ", 8)" << std::endl;
        }
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_lh

Use: Executes the LH (Load Halfword) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lh(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    try {
        // Load halfword from memory
        int16_t loaded_half = mem.get16(addr);

        // Sign-extend to 32 bits
        int32_t value = static_cast<int32_t>(loaded_half);

        // Set rd
        regs.set(rd, static_cast<uint32_t>(value));

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_load(insn, "lh");
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = sign_extend(mem[" << hex::to_hex32(addr) 
                 << "] = " << hex::to_hex0x32(static_cast<uint16_t>(loaded_half)) 
                 << ", 16)" << std::endl;
        }
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_lw

Use: Executes the LW (Load Word) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lw(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    try {
        // Load word from memory
        uint32_t loaded_word = mem.get32(addr);

        // Set rd
        regs.set(rd, loaded_word);

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_load(insn, "lw");
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = mem[" << hex::to_hex32(addr) 
                 << "] = " << hex::to_hex32(loaded_word) << std::endl;
        }
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_lbu

Use: Executes the LBU (Load Byte Unsigned) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lbu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    try {
        // Load byte from memory
        uint8_t loaded_byte = mem.get8(addr);

        // Zero-extend to 32 bits
        uint32_t value = static_cast<uint32_t>(loaded_byte);

        // Set rd
        regs.set(rd, value);

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_load(insn, "lbu");
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = zero_extend(mem[" << hex::to_hex32(addr) 
                 << "] = " << hex::to_hex0x32(loaded_byte) << ", 8)" << std::endl;
        }
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_lhu

Use: Executes the LHU (Load Halfword Unsigned) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_lhu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    try {
        // Load halfword from memory
        uint16_t loaded_half = mem.get16(addr);

        // Zero-extend to 32 bits
        uint32_t value = static_cast<uint32_t>(loaded_half);

        // Set rd
        regs.set(rd, value);

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_load(insn, "lhu");
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = zero_extend(mem[" << hex::to_hex32(addr) 
                 << "] = " << hex::to_hex0x32(loaded_half) << ", 16)" << std::endl;
        }
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sb

Use: Executes the SB (Store Byte) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sb(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Get byte to store
    uint8_t value = regs.get(rs2) & 0xFF;

    try {
        // Store byte to memory
        mem.set8(addr, value);
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_stype(insn, "sb");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// mem[" << hex::to_hex32(addr) << "] = " << hex::to_hex0x32(value) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sh

Use: Executes the SH (Store Halfword) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sh(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Get halfword to store
    uint16_t value = regs.get(rs2) & 0xFFFF;

    try {
        // Store halfword to memory
        mem.set16(addr, value);
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_stype(insn, "sh");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// mem[" << hex::to_hex32(addr) << "] = " << hex::to_hex0x32(value) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sw

Use: Executes the SW (Store Word) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sw(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Get word to store
    uint32_t value = regs.get(rs2);

    try {
        // Store word to memory
        mem.set32(addr, value);
    }
    catch(const std::out_of_range& e) {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_stype(insn, "sw");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// mem[" << hex::to_hex32(addr) << "] = " << hex::to_hex32(value) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_addi

Use: Executes the ADDI (Add Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_addi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Perform addition
    int32_t result = static_cast<int32_t>(regs.get(rs1)) + imm_i;

    // Set rd
    regs.set(rd, static_cast<uint32_t>(result));

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "addi", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " + " << imm_i 
             << " = " << hex::to_hex32(static_cast<uint32_t>(result)) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_slti

Use: Executes the SLTI (Set Less Than Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_slti(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Perform set less than
    int32_t value = (static_cast<int32_t>(regs.get(rs1)) < imm_i) ? 1 : 0;

    // Set rd
    regs.set(rd, static_cast<uint32_t>(value));

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "slti", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = (" << static_cast<int32_t>(regs.get(rs1)) 
             << " < " << imm_i << ") ? 1 : 0 = " << value << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sltiu

Use: Executes the SLTIU (Set Less Than Immediate Unsigned) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sltiu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t imm_i = decoder.get_imm_i(insn); // Treat as unsigned

    // Perform set less than unsigned
    uint32_t value = (regs.get(rs1) < static_cast<uint32_t>(imm_i)) ? 1 : 0;

    // Set rd
    regs.set(rd, value);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "sltiu", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = (" << regs.get(rs1) 
             << " < " << hex::to_hex32(imm_i) << ") ? 1 : 0 = " << value << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_xori

Use: Executes the XORI (Exclusive OR Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_xori(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t imm_i = decoder.get_imm_i(insn);

    // Perform XOR
    uint32_t result = regs.get(rs1) ^ static_cast<uint32_t>(imm_i);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "xori", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " ^ " << hex::to_hex0x32(imm_i) 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_ori

Use: Executes the ORI (OR Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ori(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t imm_i = decoder.get_imm_i(insn);

    // Perform OR
    uint32_t result = regs.get(rs1) | static_cast<uint32_t>(imm_i);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "ori", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " | " << hex::to_hex0x32(imm_i) 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_andi

Use: Executes the ANDI (AND Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_andi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t imm_i = decoder.get_imm_i(insn);

    // Perform AND
    uint32_t result = regs.get(rs1) & static_cast<uint32_t>(imm_i);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "andi", imm_i);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " & " << hex::to_hex0x32(imm_i) 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_slli

Use: Executes the SLLI (Shift Left Logical Immediate) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_slli(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t shamt = insn & 0x1F; // Shift amount [24:20]

    // Perform shift left logical
    uint32_t result = regs.get(rs1) << shamt;

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_itype_alu(insn, "slli", shamt);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " << " << shamt 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_srli_srai

Use: Executes the SRLI (Shift Right Logical Immediate) or SRAI 
     (Shift Right Arithmetic Immediate) instruction based on funct7.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_srli_srai(uint32_t insn, std::ostream* pos)
{
    uint32_t funct7 = decoder.get_funct7(insn);
    uint32_t funct3 = decoder.get_funct3(insn);
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t shamt = insn & 0x1F; // Shift amount [24:20]

    if(funct7 == rv32i_decode::funct7_srl)
    {
        // Perform shift right logical
        uint32_t result = regs.get(rs1) >> shamt;

        // Set rd
        regs.set(rd, result);

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_alu(insn, "srli", shamt);
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = x" << rs1 << " >> " << shamt 
                 << " = " << hex::to_hex32(result) << std::endl;
        }
    }
    else if(funct7 == rv32i_decode::funct7_sra)
    {
        // Perform shift right arithmetic
        int32_t value = static_cast<int32_t>(regs.get(rs1));
        int32_t result = value >> shamt;

        // Set rd
        regs.set(rd, static_cast<uint32_t>(result));

        // Optional rendering
        if(pos)
        {
            std::string s = decoder.render_itype_alu(insn, "srai", shamt);
            *pos << std::setw(35) << std::setfill(' ') << std::left << s
                 << "// x" << rd << " = x" << rs1 << " >> " << shamt 
                 << " = " << hex::to_hex32(static_cast<uint32_t>(result)) << std::endl;
        }
    }
    else
    {
        exec_illegal_insn(insn, pos);
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_add

Use: Executes the ADD (Add) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_add(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform addition
    uint32_t result = regs.get(rs1) + regs.get(rs2);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "add");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " + x" << rs2 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sub

Use: Executes the SUB (Subtract) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sub(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform subtraction
    uint32_t result = regs.get(rs1) - regs.get(rs2);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "sub");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " - x" << rs2 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sll

Use: Executes the SLL (Shift Left Logical) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sll(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform shift left logical
    uint32_t shamt = regs.get(rs2) & 0x1F; // Only lower 5 bits used
    uint32_t result = regs.get(rs1) << shamt;

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "sll");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " << " << shamt 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_slt

Use: Executes the SLT (Set Less Than) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_slt(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform set less than (signed)
    int32_t val1 = static_cast<int32_t>(regs.get(rs1));
    int32_t val2 = static_cast<int32_t>(regs.get(rs2));
    uint32_t result = (val1 < val2) ? 1 : 0;

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "slt");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = (" << val1 << " < " << val2 << ") ? 1 : 0 = " 
             << result << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sltu

Use: Executes the SLTU (Set Less Than Unsigned) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sltu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform set less than unsigned
    uint32_t val1 = regs.get(rs1);
    uint32_t val2 = regs.get(rs2);
    uint32_t result = (val1 < val2) ? 1 : 0;

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "sltu");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = (" << hex::to_hex32(val1) 
             << " < " << hex::to_hex32(val2) << ") ? 1 : 0 = " << result << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_xor

Use: Executes the XOR (Exclusive OR) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_xor(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform XOR
    uint32_t result = regs.get(rs1) ^ regs.get(rs2);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "xor");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " ^ x" << rs2 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_srl

Use: Executes the SRL (Shift Right Logical) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_srl(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform shift right logical
    uint32_t shamt = regs.get(rs2) & 0x1F; // Only lower 5 bits
    uint32_t result = regs.get(rs1) >> shamt;

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "srl");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " >> " << shamt 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_sra

Use: Executes the SRA (Shift Right Arithmetic) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_sra(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform shift right arithmetic
    uint32_t shamt = regs.get(rs2) & 0x1F; // Only lower 5 bits
    int32_t value = static_cast<int32_t>(regs.get(rs1));
    int32_t result = value >> shamt;

    // Set rd
    regs.set(rd, static_cast<uint32_t>(result));

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "sra");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " >> " << shamt 
             << " = " << hex::to_hex32(static_cast<uint32_t>(result)) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_or

Use: Executes the OR (Logical OR) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_or(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform OR
    uint32_t result = regs.get(rs1) | regs.get(rs2);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "or");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " | x" << rs2 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_and

Use: Executes the AND (Logical AND) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_and(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);

    // Perform AND
    uint32_t result = regs.get(rs1) & regs.get(rs2);

    // Set rd
    regs.set(rd, result);

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_rtype(insn, "and");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = x" << rs1 << " & x" << rs2 
             << " = " << hex::to_hex32(result) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_system

Use: Executes system instructions like ECALL, EBREAK, and CSR operations.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_system(uint32_t insn, std::ostream* pos)
{
    uint32_t imm = (insn >> 20) & 0xFFF; // Immediate field for system instructions
    uint32_t funct3 = decoder.get_funct3(insn);

    if(imm == rv32i_decode::ecall_imm && funct3 == 0x0)
    {
        exec_ecall(insn, pos);
    }
    else if(imm == rv32i_decode::ebreak_imm && funct3 == 0x0)
    {
        exec_ebreak(insn, pos);
    }
    else if(funct3 == rv32i_decode::funct3_csrrw ||
            funct3 == rv32i_decode::funct3_csrrs ||
            funct3 == rv32i_decode::funct3_csrrc ||
            funct3 == rv32i_decode::funct3_csrrwi ||
            funct3 == rv32i_decode::funct3_csrrsi ||
            funct3 == rv32i_decode::funct3_csrrci)
    {
        // Handle CSR operations
        if(funct3 == rv32i_decode::funct3_csrrw ||
           funct3 == rv32i_decode::funct3_csrrs ||
           funct3 == rv32i_decode::funct3_csrrc)
        {
            exec_csrrx(insn, pos);
        }
        else if(funct3 == rv32i_decode::funct3_csrrwi ||
                funct3 == rv32i_decode::funct3_csrrsi ||
                funct3 == rv32i_decode::funct3_csrrci)
        {
            exec_csrrxi(insn, pos);
        }
        else
        {
            exec_illegal_insn(insn, pos);
        }
    }
    else
    {
        exec_illegal_insn(insn, pos);
    }
}

/**********************************************************************
Function: exec_csrrx

Use: Executes CSR Read/Write, Read/Set, and Read/Clear instructions.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrx(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t csr = (insn >> 20) & 0xFFF; // CSR address

    // Determine the CSR operation mnemonic based on funct3
    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrw)
        mnemonic = "csrrw";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrs)
        mnemonic = "csrrs";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrc)
        mnemonic = "csrrc";
    else
        mnemonic = "unknown";

    // Check if CSR exists
    auto csr_it = csr_map.find(csr);
    if(csr_it == csr_map.end())
    {
        // Undefined CSR
        exec_illegal_insn(insn, pos);
        return;
    }

    uint32_t csr_val = csr_it->second;

    uint32_t new_csr_val = csr_val;
    uint32_t rs1_val = regs.get(rs1);

    if(mnemonic == "csrrw")
    {
        // Swap CSR and rs1
        new_csr_val = rs1_val;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrs")
    {
        // Set CSR = CSR | rs1
        new_csr_val = csr_val | rs1_val;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrc")
    {
        // Set CSR = CSR & ~rs1
        new_csr_val = csr_val & ~rs1_val;
        regs.set(rd, csr_val);
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Update CSR
    csr_map[csr] = new_csr_val;

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_csrrx(insn, mnemonic);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = " << hex::to_hex32(csr_val) 
             << ", CSR[" << hex::to_hex32(csr) << "] = " << hex::to_hex32(new_csr_val) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_csrrxi

Use: Executes CSR Read/Write Immediate, Read/Set Immediate, and 
     Read/Clear Immediate instructions.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrxi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t zimm = decoder.get_rs1(insn); // Zero-extended immediate
    uint32_t csr = (insn >> 20) & 0xFFF;  // CSR address

    // Determine the CSR operation mnemonic based on funct3
    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrwi)
        mnemonic = "csrrwi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrsi)
        mnemonic = "csrrsi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrci)
        mnemonic = "csrrci";
    else
        mnemonic = "unknown";

    // Check if CSR exists
    auto csr_it = csr_map.find(csr);
    if(csr_it == csr_map.end())
    {
        // Undefined CSR
        exec_illegal_insn(insn, pos);
        return;
    }

    uint32_t csr_val = csr_it->second;

    uint32_t new_csr_val = csr_val;

    if(mnemonic == "csrrwi")
    {
        // Swap CSR and zimm (treated as rs1 = zimm)
        new_csr_val = zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrsi")
    {
        // Set CSR = CSR | zimm
        new_csr_val = csr_val | zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrci")
    {
        // Set CSR = CSR & ~zimm
        new_csr_val = csr_val & ~zimm;
        regs.set(rd, csr_val);
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Update CSR
    csr_map[csr] = new_csr_val;

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_csrrxi(insn, mnemonic);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = " << hex::to_hex32(csr_val) 
             << ", CSR[" << hex::to_hex32(csr) << "] = " << hex::to_hex32(new_csr_val) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_ecall

Use: Executes the ECALL (Environment Call) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ecall(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ecall");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment call" << std::endl;
    }

    // Define behavior for ECALL, e.g., syscall handling, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "ECALL instruction";
}

/**********************************************************************
Function: exec_ebreak

Use: Executes the EBREAK (Environment Break) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ebreak(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ebreak");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment break" << std::endl;
    }

    // Define behavior for EBREAK, e.g., debug breakpoint, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "EBREAK instruction";
}

/**********************************************************************
Function: exec_illegal_insn

Use: Handles illegal or unimplemented instructions by halting the simulator
     with an error message.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_illegal_insn(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_illegal_insn();
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Illegal instruction: " << hex::to_hex0x32(insn) << std::endl;
    }

    // Halt the simulator with an error
    halt = true;
    halt_reason = "Illegal instruction encountered";
}

/**********************************************************************
Function: exec_csrrw

Use: Executes the CSRRW (CSR Read and Write) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrw(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrs

Use: Executes the CSRRS (CSR Read and Set) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrs(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrc

Use: Executes the CSRRC (CSR Read and Clear) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrc(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrwi

Use: Executes the CSRRWI (CSR Read and Write Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrwi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrsi

Use: Executes the CSRRSI (CSR Read and Set Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrsi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrci

Use: Executes the CSRRCI (CSR Read and Clear Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrci(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrxi

Use: Executes CSR Read/Write Immediate, Read/Set Immediate, and 
     Read/Clear Immediate instructions.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrxi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t zimm = decoder.get_rs1(insn); // Zero-extended immediate
    uint32_t csr = (insn >> 20) & 0xFFF;  // CSR address

    // Determine the CSR operation mnemonic based on funct3
    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrwi)
        mnemonic = "csrrwi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrsi)
        mnemonic = "csrrsi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrci)
        mnemonic = "csrrci";
    else
        mnemonic = "unknown";

    // Check if CSR exists
    auto csr_it = csr_map.find(csr);
    if(csr_it == csr_map.end())
    {
        // Undefined CSR
        exec_illegal_insn(insn, pos);
        return;
    }

    uint32_t csr_val = csr_it->second;
    uint32_t new_csr_val = csr_val;

    if(mnemonic == "csrrwi")
    {
        // Swap CSR and zimm (treated as rs1 = zimm)
        new_csr_val = zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrsi")
    {
        // Set CSR = CSR | zimm
        new_csr_val = csr_val | zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrci")
    {
        // Set CSR = CSR & ~zimm
        new_csr_val = csr_val & ~zimm;
        regs.set(rd, csr_val);
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Update CSR
    csr_map[csr] = new_csr_val;

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_csrrxi(insn, mnemonic);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = " << hex::to_hex32(csr_val) 
             << ", CSR[" << hex::to_hex32(csr) << "] = " << hex::to_hex32(new_csr_val) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_ecall

Use: Executes the ECALL (Environment Call) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ecall(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ecall");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment call" << std::endl;
    }

    // Define behavior for ECALL, e.g., syscall handling, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "ECALL instruction";
}

/**********************************************************************
Function: exec_ebreak

Use: Executes the EBREAK (Environment Break) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ebreak(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ebreak");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment break" << std::endl;
    }

    // Define behavior for EBREAK, e.g., debug breakpoint, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "EBREAK instruction";
}

/**********************************************************************
Function: exec_illegal_insn

Use: Handles illegal or unimplemented instructions by halting the simulator
     with an error message.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_illegal_insn(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_illegal_insn();
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Illegal instruction: " << hex::to_hex0x32(insn) << std::endl;
    }

    // Halt the simulator with an error
    halt = true;
    halt_reason = "Illegal instruction encountered";
}

/**********************************************************************
Function: exec_csrrw

Use: Executes the CSRRW (CSR Read and Write) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrw(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrs

Use: Executes the CSRRS (CSR Read and Set) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrs(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrc

Use: Executes the CSRRC (CSR Read and Clear) instruction.
     Delegates to exec_csrrx.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrc(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/**********************************************************************
Function: exec_csrrwi

Use: Executes the CSRRWI (CSR Read and Write Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrwi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrsi

Use: Executes the CSRRSI (CSR Read and Set Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrsi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrci

Use: Executes the CSRRCI (CSR Read and Clear Immediate) instruction.
     Delegates to exec_csrrxi.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrci(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/**********************************************************************
Function: exec_csrrxi

Use: Executes CSR Read/Write Immediate, Read/Set Immediate, and 
     Read/Clear Immediate instructions.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_csrrxi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t zimm = decoder.get_rs1(insn); // Zero-extended immediate
    uint32_t csr = (insn >> 20) & 0xFFF;  // CSR address

    // Determine the CSR operation mnemonic based on funct3
    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrwi)
        mnemonic = "csrrwi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrsi)
        mnemonic = "csrrsi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrci)
        mnemonic = "csrrci";
    else
        mnemonic = "unknown";

    // Check if CSR exists
    auto csr_it = csr_map.find(csr);
    if(csr_it == csr_map.end())
    {
        // Undefined CSR
        exec_illegal_insn(insn, pos);
        return;
    }

    uint32_t csr_val = csr_it->second;
    uint32_t new_csr_val = csr_val;

    if(mnemonic == "csrrwi")
    {
        // Swap CSR and zimm (treated as rs1 = zimm)
        new_csr_val = zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrsi")
    {
        // Set CSR = CSR | zimm
        new_csr_val = csr_val | zimm;
        regs.set(rd, csr_val);
    }
    else if(mnemonic == "csrrci")
    {
        // Set CSR = CSR & ~zimm
        new_csr_val = csr_val & ~zimm;
        regs.set(rd, csr_val);
    }
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Update CSR
    csr_map[csr] = new_csr_val;

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_csrrxi(insn, mnemonic);
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// x" << rd << " = " << hex::to_hex32(csr_val) 
             << ", CSR[" << hex::to_hex32(csr) << "] = " << hex::to_hex32(new_csr_val) << std::endl;
    }

    // Increment PC
    pc += 4;
}

/**********************************************************************
Function: exec_ecall

Use: Executes the ECALL (Environment Call) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ecall(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ecall");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment call" << std::endl;
    }

    // Define behavior for ECALL, e.g., syscall handling, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "ECALL instruction";
}

/**********************************************************************
Function: exec_ebreak

Use: Executes the EBREAK (Environment Break) instruction.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_ebreak(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_mnemonic("ebreak");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Environment break" << std::endl;
    }

    // Define behavior for EBREAK, e.g., debug breakpoint, halt simulation, etc.
    // For simplicity, we'll halt the simulator
    halt = true;
    halt_reason = "EBREAK instruction";
}

/**********************************************************************
Function: exec_illegal_insn

Use: Handles illegal or unimplemented instructions by halting the simulator
     with an error message.

Arguments:
1. insn: The binary representation of the instruction.
2. pos: Optional output stream for logging/disassembly.

Returns: void
**********************************************************************/
void rv32i_hart::exec_illegal_insn(uint32_t insn, std::ostream* pos)
{
    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_illegal_insn();
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// Illegal instruction: " << hex::to_hex0x32(insn) << std::endl;
    }

    // Halt the simulator with an error
    halt = true;
    halt_reason = "Illegal instruction encountered";
}

/**********************************************************************
Function: halt_simulator

Use: Halts the simulator with a specified reason.

Arguments:
1. reason: The reason for halting the simulator.

Returns: void
**********************************************************************/
void rv32i_hart::halt_simulator(const std::string& reason)
{
    halt = true;
    halt_reason = reason;
}

// Implement other necessary member functions if any (e.g., getters)

/**********************************************************************
Function: is_halted

Use: Checks if the simulator has been halted.

Arguments: None

Returns:
- bool: true if halted, false otherwise.
**********************************************************************/
bool rv32i_hart::is_halted() const
{
    return halt;
}

/**********************************************************************
Function: get_halt_reason

Use: Retrieves the reason why the simulator was halted.

Arguments: None

Returns:
- std::string: The halt reason.
**********************************************************************/
std::string rv32i_hart::get_halt_reason() const
{
    return halt_reason;
}

/**********************************************************************
Function: get_pc

Use: Retrieves the current program counter.

Arguments: None

Returns:
- uint32_t: The current PC value.
**********************************************************************/
uint32_t rv32i_hart::get_pc() const
{
    return pc;
}

