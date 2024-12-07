//*************************************************************************
// Ryan Scaglione
// z1996413
// CSCI463 - PE1
//
// I certify that this is my own work, and where applicable an extension
// of the starter code for the assignment
//
//*************************************************************************

#ifndef RV32I_HART_H
#define RV32I_HART_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <iostream>

// Forward declarations of dependent classes
class rv32i_decode;
class registerfile;
class memory;

// Namespace for hexadecimal utility functions
namespace hex {
    // Assume these functions are defined in hex.h/cpp
    std::string to_hex0x32(uint32_t val);
    std::string to_hex0x16(uint16_t val);
    std::string to_hex0x8(uint8_t val);
    std::string to_hex32(uint32_t val);
}

// rv32i_hart Class Definition
class rv32i_hart {
public:
    // Constructor
    rv32i_hart();

    // Main execution function
    void exec(uint32_t insn, std::ostream* pos = nullptr);

    // Execution functions for each instruction type
    // U-Type Instructions
    void exec_lui(uint32_t insn, std::ostream* pos);
    void exec_auipc(uint32_t insn, std::ostream* pos);

    // J-Type Instruction
    void exec_jal(uint32_t insn, std::ostream* pos);

    // I-Type Instructions
    void exec_jalr(uint32_t insn, std::ostream* pos);
    void exec_lb(uint32_t insn, std::ostream* pos);
    void exec_lh(uint32_t insn, std::ostream* pos);
    void exec_lw(uint32_t insn, std::ostream* pos);
    void exec_lbu(uint32_t insn, std::ostream* pos);
    void exec_lhu(uint32_t insn, std::ostream* pos);
    void exec_sb(uint32_t insn, std::ostream* pos);
    void exec_sh(uint32_t insn, std::ostream* pos);
    void exec_sw(uint32_t insn, std::ostream* pos);
    void exec_addi(uint32_t insn, std::ostream* pos);
    void exec_slti(uint32_t insn, std::ostream* pos);
    void exec_sltiu(uint32_t insn, std::ostream* pos);
    void exec_xori(uint32_t insn, std::ostream* pos);
    void exec_ori(uint32_t insn, std::ostream* pos);
    void exec_andi(uint32_t insn, std::ostream* pos);
    void exec_slli(uint32_t insn, std::ostream* pos);
    void exec_srli_srai(uint32_t insn, std::ostream* pos);

    // R-Type Instructions
    void exec_add(uint32_t insn, std::ostream* pos);
    void exec_sub(uint32_t insn, std::ostream* pos);
    void exec_sll(uint32_t insn, std::ostream* pos);
    void exec_slt(uint32_t insn, std::ostream* pos);
    void exec_sltu(uint32_t insn, std::ostream* pos);
    void exec_xor(uint32_t insn, std::ostream* pos);
    void exec_srl(uint32_t insn, std::ostream* pos);
    void exec_sra(uint32_t insn, std::ostream* pos);
    void exec_or(uint32_t insn, std::ostream* pos);
    void exec_and(uint32_t insn, std::ostream* pos);

    // B-Type Instructions (Branches)
    void exec_beq(uint32_t insn, std::ostream* pos);
    void exec_bne(uint32_t insn, std::ostream* pos);
    void exec_blt(uint32_t insn, std::ostream* pos);
    void exec_bge(uint32_t insn, std::ostream* pos);
    void exec_bltu(uint32_t insn, std::ostream* pos);
    void exec_bgeu(uint32_t insn, std::ostream* pos);

    // System Instructions
    void exec_ecall(uint32_t insn, std::ostream* pos);
    void exec_ebreak(uint32_t insn, std::ostream* pos);
    void exec_system(uint32_t insn, std::ostream* pos);

    // CSR Instructions
    void exec_csrrx(uint32_t insn, std::ostream* pos);
    void exec_csrrxi(uint32_t insn, std::ostream* pos);
    void exec_csrrw(uint32_t insn, std::ostream* pos);   // Delegates to exec_csrrx
    void exec_csrrs(uint32_t insn, std::ostream* pos);   // Delegates to exec_csrrx
    void exec_csrrc(uint32_t insn, std::ostream* pos);   // Delegates to exec_csrrx
    void exec_csrrwi(uint32_t insn, std::ostream* pos);  // Delegates to exec_csrrxi
    void exec_csrrsi(uint32_t insn, std::ostream* pos);  // Delegates to exec_csrrxi
    void exec_csrrci(uint32_t insn, std::ostream* pos);  // Delegates to exec_csrrxi

    // Illegal Instruction Handler
    void exec_illegal_insn(uint32_t insn, std::ostream* pos);

    // Simulator Control
    void halt_simulator(const std::string& reason);

    // Accessors
    uint32_t get_pc() const { return pc; }
    bool is_halted() const { return halt; }
    std::string get_halt_reason() const { return halt_reason; }

private:
    // Member Variables
    uint32_t pc;  // Program Counter
    bool halt;    // Halt flag
    std::string halt_reason; // Reason for halting

    rv32i_decode decoder;    // Instruction decoder
    registerfile regs;       // Register file
    memory mem;               // Memory

    std::unordered_map<uint32_t, uint32_t> csr_map; // CSR address to value mapping
};

#endif // RV32I_HART_H