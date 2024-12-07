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
rv32i_hart.cpp

Implementation of the rv32i_hart class, which simulates a single RISC-V RV32I hart.
Handles instruction execution, state management, and simulation controls.

*************************************************************************/

#include "rv32i_hart.h"
#include "rv32i_decode.h"
#include "registerfile.h"
#include "memory.h"
#include "hex.h"
#include <iostream>
#include <iomanip>
#include <cctype>

// Constructor: Initializes the CSR map and other necessary components
/*************************************************************************
Function: rv32i_hart

Use: Constructs a new rv32i_hart object, initializing PC, halt flags, and CSR map.

Arguments: None

 ************************************************************************/
rv32i_hart::rv32i_hart()
    : pc(0), halt(false), halt_reason("none"), decoder(), regs(), mem(),
      insn_counter(0), mhartid(0), show_instructions(false), show_registers(false)
{
    // Initialize CSR map with standard CSRs (modify as needed)
    csr_map[0x300] = 0; // mhartid
    csr_map[0x301] = 0; // mstatus
    csr_map[0x342] = 0; // mtvec
    // Add other CSRs as necessary
}

/*************************************************************************
Function: reset

Use: Resets the rv32i_hart object and the registerfile.

Arguments: None

 ************************************************************************/
void rv32i_hart::reset()
{
    pc = 0;
    regs.reset();
    insn_counter = 0;
    halt = false;
    halt_reason = "none";
    // Optionally reset CSR map if needed
}

/*************************************************************************
Function: dump

Use: Dumps the entire state of the hart. Prefix each line printed by the given hdr
     string (the default being to not print any prefix.) It will dump the GP-regs
     and then add a dump of the PC register.

Arguments:
1. const std::string &hdr: Optional header string to prefix each line.

 ************************************************************************/
void rv32i_hart::dump(const std::string &hdr) const
{
    regs.dump(hdr);
    std::cout << hdr << "pc " << hex::to_hex32(pc) << std::endl;
}

/*************************************************************************
Function: get_insn_counter

Use: Accessor for insn_counter. Returns the number of instructions that have been
     executed by the simulator since the last reset().

Arguments: None

Returns: uint64_t: The instruction counter value.

 ************************************************************************/
uint64_t rv32i_hart::get_insn_counter() const
{
    return insn_counter;
}

/*************************************************************************
Function: set_mhartid

Use: Mutator for mhartid. Sets the ID value to be returned by the csrrs instruction
     for CSR register number 0xf14.

Arguments:
1. int i: The hart ID to set.

 ************************************************************************/
void rv32i_hart::set_mhartid(int i)
{
    mhartid = i;
    csr_map[0xf14] = mhartid; // Assuming 0xf14 is the CSR for mhartid
}

/*************************************************************************
Function: tick

Use: Executes a single instruction cycle. If the hart is halted, returns immediately.
     Otherwise, fetches, decodes, and executes an instruction, with optional rendering
     based on flags.

Arguments:
1. const std::string &hdr: Prefix string for any output.

 ************************************************************************/
void rv32i_hart::tick(const std::string &hdr)
{
    if(halt) {
        return;
    }

    // Check PC alignment
    if(pc % 4 != 0) {
        halt_simulator("PC alignment error");
        return;
    }

    // Fetch instruction from memory
    uint32_t insn = mem.get32(pc);

    // Increment instruction counter
    insn_counter++;

    // Show registers before execution if flag is set
    if(show_registers) {
        dump(hdr);
    }

    // Show instruction if flag is set
    if(show_instructions) {
        std::cout << hex::to_hex32(pc) << ": " 
                  << hex::to_hex0x32(insn) << " ";
        exec(insn, &std::cout);
    }
    else {
        exec(insn, nullptr);
    }
}

/*************************************************************************
Function: exec

Use: Executes the given RV32I instruction by decoding it and invoking the associated
     exec_xxx() helper function.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec(uint32_t insn, std::ostream* pos)
{
    uint32_t opcode = decoder.get_opcode(insn);

    switch (opcode)
    {
        // U-Type Instructions
        case rv32i_decode::opcode_lui:
            exec_lui(insn, pos);
            break;

        case rv32i_decode::opcode_auipc:
            exec_auipc(insn, pos);
            break;

        // J-Type Instruction
        case rv32i_decode::opcode_jal:
            exec_jal(insn, pos);
            break;

        // I-Type Instructions
        case rv32i_decode::opcode_jalr:
            exec_jalr(insn, pos);
            break;

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
            break;

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
            break;

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
            break;

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
            break;

        case rv32i_decode::opcode_system:
            exec_system(insn, pos);
            break;

        default:
            exec_illegal_insn(insn, pos);
            break;
    }

    // Increment PC if not modified by instruction
    // Note: Some instructions like branches and jumps modify PC
    // Others increment PC by 4. Ensure that all exec_xxx functions handle PC.
}

/*************************************************************************
Function: exec_illegal_insn

Use: Handles illegal or unimplemented instructions by halting the simulator
     with an error message.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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
    halt_simulator("Illegal instruction encountered");
}

/*************************************************************************
Function: halt_simulator

Use: Halts the simulator with a specified reason.

Arguments:
1. const std::string& reason: The reason for halting the simulator.

 ************************************************************************/
void rv32i_hart::halt_simulator(const std::string& reason)
{
    halt = true;
    halt_reason = reason;
}

/*************************************************************************
Function: reset

Use: Resets the rv32i_hart object and the registerfile.

Arguments: None

 ************************************************************************/
void rv32i_hart::reset()
{
    pc = 0;
    regs.reset();
    insn_counter = 0;
    halt = false;
    halt_reason = "none";
    // Optionally reset CSR map if needed
}

/*************************************************************************
Function: dump

Use: Dumps the entire state of the hart. Prefix each line printed by the given hdr
     string (the default being to not print any prefix.) It will dump the GP-regs
     and then add a dump of the PC register.

Arguments:
1. const std::string &hdr: Optional header string to prefix each line.

 ************************************************************************/
void rv32i_hart::dump(const std::string &hdr) const
{
    regs.dump(hdr);
    std::cout << hdr << "pc " << hex::to_hex32(pc) << std::endl;
}

/*************************************************************************
Function: get_insn_counter

Use: Accessor for insn_counter. Returns the number of instructions that have been
     executed by the simulator since the last reset().

Arguments: None

Returns: uint64_t: The instruction counter value.

 ************************************************************************/
uint64_t rv32i_hart::get_insn_counter() const
{
    return insn_counter;
}

/*************************************************************************
Function: set_mhartid

Use: Mutator for mhartid. Sets the ID value to be returned by the csrrs instruction
     for CSR register number 0xf14.

Arguments:
1. int i: The hart ID to set.

 ************************************************************************/
void rv32i_hart::set_mhartid(int i)
{
    mhartid = i;
    csr_map[0xf14] = mhartid; // Assuming 0xf14 is the CSR for mhartid
}

/*************************************************************************
Function: tick

Use: Executes a single instruction cycle. If the hart is halted, returns immediately.
     Otherwise, fetches, decodes, and executes an instruction, with optional rendering
     based on flags.

Arguments:
1. const std::string &hdr: Prefix string for any output.

 ************************************************************************/
void rv32i_hart::tick(const std::string &hdr)
{
    if(halt) {
        return;
    }

    // Check PC alignment
    if(pc % 4 != 0) {
        halt_simulator("PC alignment error");
        return;
    }

    // Fetch instruction from memory
    uint32_t insn = mem.get32(pc);

    // Increment instruction counter
    insn_counter++;

    // Show registers before execution if flag is set
    if(show_registers) {
        dump(hdr);
    }

    // Show instruction if flag is set
    if(show_instructions) {
        std::cout << hex::to_hex32(pc) << ": " 
                  << hex::to_hex0x32(insn) << " ";
        exec(insn, &std::cout);
    }
    else {
        exec(insn, nullptr);
    }
}

/*************************************************************************
Function: exec_lui

Use: Executes the LUI (Load Upper Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_auipc

Use: Executes the AUIPC (Add Upper Immediate to PC) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_jal

Use: Executes the JAL (Jump and Link) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_jalr

Use: Executes the JALR (Jump and Link Register) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_jalr(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate target address: (rs1 + imm) & ~1
    uint32_t target = (regs.get(rs1) + imm_i) & ~1;

    // Save PC + 4 to rd
    regs.set(rd, pc + 4);

   

    // Jump to target address
    pc = target;
}

/*************************************************************************
Function: exec_lb

Use: Executes the LB (Load Byte) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_lb(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 1))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Load byte from memory
    int8_t loaded_byte = mem.get8(addr);

    // Sign-extend to 32 bits
    int32_t value = static_cast<int32_t>(loaded_byte);

    // Set rd
    regs.set(rd, static_cast<uint32_t>(value));

   

    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_lh

Use: Executes the LH (Load Halfword) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_lh(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 2))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Load halfword from memory
    int16_t loaded_half = mem.get16(addr);

    // Sign-extend to 32 bits
    int32_t value = static_cast<int32_t>(loaded_half);

    // Set rd
    regs.set(rd, static_cast<uint32_t>(value));

   

    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_lw

Use: Executes the LW (Load Word) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_lw(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 4))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Load word from memory
    uint32_t loaded_word = mem.get32(addr);

    // Set rd
    regs.set(rd, loaded_word);

    
    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_lbu

Use: Executes the LBU (Load Byte Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_lbu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 1))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Load byte from memory
    uint8_t loaded_byte = mem.get8(addr);

    // Zero-extend to 32 bits
    uint32_t value = static_cast<uint32_t>(loaded_byte);

    // Set rd
    regs.set(rd, value);

    
    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_lhu

Use: Executes the LHU (Load Halfword Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_lhu(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    int32_t imm_i = decoder.get_imm_i(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_i;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 2))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Load halfword from memory
    uint16_t loaded_half = mem.get16(addr);

    // Zero-extend to 32 bits
    uint32_t value = static_cast<uint32_t>(loaded_half);

    // Set rd
    regs.set(rd, value);

    
    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_sb

Use: Executes the SB (Store Byte) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_sb(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 1))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Get byte to store
    uint8_t value = regs.get(rs2) & 0xFF;

    // Store byte to memory
    mem.set8(addr, value);

   
    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_sh

Use: Executes the SH (Store Halfword) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_sh(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 2))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Get halfword to store
    uint16_t value = regs.get(rs2) & 0xFFFF;

    // Store halfword to memory
    mem.set16(addr, value);

   
    // Increment PC
    pc += 4;
}

/*************************************************************************
Function: exec_sw

Use: Executes the SW (Store Word) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_sw(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_s = decoder.get_imm_s(insn);

    // Calculate address
    uint32_t addr = regs.get(rs1) + imm_s;

    // Check for illegal memory access
    if(mem.check_illegal(addr, 4))
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Get word to store
    uint32_t value = regs.get(rs2);

    // Store word to memory
    mem.set32(addr, value);

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

/*************************************************************************
Function: exec_addi

Use: Executes the ADDI (Add Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_slti

Use: Executes the SLTI (Set Less Than Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_sltiu

Use: Executes the SLTIU (Set Less Than Immediate Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_xori

Use: Executes the XORI (Exclusive OR Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_ori

Use: Executes the ORI (OR Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_andi

Use: Executes the ANDI (AND Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_slli

Use: Executes the SLLI (Shift Left Logical Immediate) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_srli_srai

Use: Executes the SRLI (Shift Right Logical Immediate) or SRAI 
     (Shift Right Arithmetic Immediate) instruction based on funct7.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_srli_srai(uint32_t insn, std::ostream* pos)
{
    uint32_t funct7 = decoder.get_funct7(insn);
    uint32_t funct3 = decoder.get_funct3(insn);
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t shamt = insn & 0x1F; // Shift amount [24:20]

    if(funct7 == rv32i_decode::funct7_srli)
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
    else if(funct7 == rv32i_decode::funct7_srai)
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

/*************************************************************************
Function: exec_add

Use: Executes the ADD (Add) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_sub

Use: Executes the SUB (Subtract) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_sll

Use: Executes the SLL (Shift Left Logical) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_slt

Use: Executes the SLT (Set Less Than) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_sltu

Use: Executes the SLTU (Set Less Than Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_xor

Use: Executes the XOR (Exclusive OR) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_srl

Use: Executes the SRL (Shift Right Logical) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_sra

Use: Executes the SRA (Shift Right Arithmetic) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_or

Use: Executes the OR (Logical OR) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_and

Use: Executes the AND (Logical AND) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_beq

Use: Executes the BEQ (Branch if Equal) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_beq(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Check if registers are equal
    bool condition = (regs.get(rs1) == regs.get(rs2));

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "beq");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// " << (condition ? "branch taken" : "branch not taken") << std::endl;
    }
}

/*************************************************************************
Function: exec_bne

Use: Executes the BNE (Branch if Not Equal) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_bne(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Check if registers are not equal
    bool condition = (regs.get(rs1) != regs.get(rs2));

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "bne");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// " << (condition ? "branch taken" : "branch not taken") << std::endl;
    }
}

/*************************************************************************
Function: exec_blt

Use: Executes the BLT (Branch if Less Than) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_blt(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Compare as signed integers
    int32_t val1 = static_cast<int32_t>(regs.get(rs1));
    int32_t val2 = static_cast<int32_t>(regs.get(rs2));
    bool condition = (val1 < val2);

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "blt");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// (" << val1 << " < " << val2 << ") = " << condition << std::endl;
    }
}

/*************************************************************************
Function: exec_bge

Use: Executes the BGE (Branch if Greater or Equal) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_bge(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Compare as signed integers
    int32_t val1 = static_cast<int32_t>(regs.get(rs1));
    int32_t val2 = static_cast<int32_t>(regs.get(rs2));
    bool condition = (val1 >= val2);

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "bge");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// (" << val1 << " >= " << val2 << ") = " << condition << std::endl;
    }
}

/*************************************************************************
Function: exec_bltu

Use: Executes the BLTU (Branch if Less Than Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_bltu(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Compare as unsigned integers
    uint32_t val1 = regs.get(rs1);
    uint32_t val2 = regs.get(rs2);
    bool condition = (val1 < val2);

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "bltu");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// (" << hex::to_hex32(val1) << " <U " << hex::to_hex32(val2) 
             << ") = " << condition << std::endl;
    }
}

/*************************************************************************
Function: exec_bgeu

Use: Executes the BGEU (Branch if Greater or Equal Unsigned) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_bgeu(uint32_t insn, std::ostream* pos)
{
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t rs2 = decoder.get_rs2(insn);
    int32_t imm_b = decoder.get_imm_b(insn);

    // Compare as unsigned integers
    uint32_t val1 = regs.get(rs1);
    uint32_t val2 = regs.get(rs2);
    bool condition = (val1 >= val2);

    if(condition)
    {
        // Branch taken
        pc += imm_b;
    }
    else
    {
        // Branch not taken
        pc += 4;
    }

    // Optional rendering
    if(pos)
    {
        std::string s = decoder.render_btype(pc, insn, "bgeu");
        *pos << std::setw(35) << std::setfill(' ') << std::left << s
             << "// (" << hex::to_hex32(val1) << " >=U " << hex::to_hex32(val2) 
             << ") = " << condition << std::endl;
    }
}

/*************************************************************************
Function: exec_ecall

Use: Executes the ECALL (Environment Call) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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
    halt_simulator("ECALL instruction");
}

/*************************************************************************
Function: exec_ebreak

Use: Executes the EBREAK (Environment Break) instruction.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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
    halt_simulator("EBREAK instruction");
}

/*************************************************************************
Function: exec_system

Use: Executes system instructions like ECALL, EBREAK, and CSR operations.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
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

/*************************************************************************
Function: exec_csrrx

Use: Executes CSR Read/Write, Read/Set, and Read/Clear instructions.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrx(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t rs1 = decoder.get_rs1(insn);
    uint32_t csr = (insn >> 20) & 0xFFF; // CSR address

    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrw)
        mnemonic = "csrrw";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrs)
        mnemonic = "csrrs";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrc)
        mnemonic = "csrrc";

    // Retrieve current CSR value; default to 0 if not present
    uint32_t csr_val = (csr_map.find(csr) != csr_map.end()) ? csr_map[csr] : 0;

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

/*************************************************************************
Function: exec_csrrxi

Use: Executes CSR Read/Write Immediate, Read/Set Immediate, and 
     Read/Clear Immediate instructions.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrxi(uint32_t insn, std::ostream* pos)
{
    uint32_t rd = decoder.get_rd(insn);
    uint32_t zimm = decoder.get_rs1(insn); // Zero-extended immediate
    uint32_t csr = (insn >> 20) & 0xFFF;  // CSR address

    std::string mnemonic;
    if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrwi)
        mnemonic = "csrrwi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrsi)
        mnemonic = "csrrsi";
    else if(decoder.get_funct3(insn) == rv32i_decode::funct3_csrrci)
        mnemonic = "csrrci";
    else
    {
        exec_illegal_insn(insn, pos);
        return;
    }

    // Retrieve current CSR value; default to 0 if not present
    uint32_t csr_val = (csr_map.find(csr) != csr_map.end()) ? csr_map[csr] : 0;

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

/*************************************************************************
Function: exec_csrrw

Use: Executes the CSR Read/Write instruction by delegating to exec_csrrx.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrw(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/*************************************************************************
Function: exec_csrrs

Use: Executes the CSR Read/Set instruction by delegating to exec_csrrx.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrs(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/*************************************************************************
Function: exec_csrrc

Use: Executes the CSR Read/Clear instruction by delegating to exec_csrrx.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrc(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrx
    exec_csrrx(insn, pos);
}

/*************************************************************************
Function: exec_csrrwi

Use: Executes the CSR Read/Write Immediate instruction by delegating to exec_csrrxi.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrwi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/*************************************************************************
Function: exec_csrrsi

Use: Executes the CSR Read/Set Immediate instruction by delegating to exec_csrrxi.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrsi(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}

/*************************************************************************
Function: exec_csrrci

Use: Executes the CSR Read/Clear Immediate instruction by delegating to exec_csrrxi.

Arguments:
1. uint32_t insn: The binary representation of the instruction.
2. std::ostream* pos: Optional output stream for logging/disassembly.

 ************************************************************************/
void rv32i_hart::exec_csrrci(uint32_t insn, std::ostream* pos)
{
    // Delegates to exec_csrrxi
    exec_csrrxi(insn, pos);
}
