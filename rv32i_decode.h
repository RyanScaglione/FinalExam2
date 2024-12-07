//*************************************************************************
//Ryan Scaglione
//z1996413
//CSCI463 - PE1
//
//
//I certify that this is my own work, and where applicable an extension
//of the starter code for the assignment
//Due date: 11/22/2024
//
//  
//
//*************************************************************************

#ifndef RV32I_DECODE_H
#define RV32I_DECODE_H

#include <string>
#include <cstdint>

class rv32i_decode
{
public:
    static const uint32_t XLEN = 32;
    static const int mnemonic_width = 8;

    //opcodes
    static const uint32_t opcode_lui     = 0x37; 

     static const uint32_t opcode_auipc   = 0x17; 
    static const uint32_t opcode_jal     = 0x6f; 
    static const uint32_t opcode_jalr    = 0x67; 
    static const uint32_t opcode_branch  = 0x63;
    static const uint32_t opcode_load    = 0x03; 
    static const uint32_t opcode_store   = 0x23; 
    static const uint32_t opcode_alu_imm = 0x13; 
    static const uint32_t opcode_alu_reg = 0x33; 
    static const uint32_t opcode_fence   = 0x0f; 
    static const uint32_t opcode_system  = 0x73; 

    //funct3 values branch
     static const uint32_t funct3_beq  = 0x0; 
    static const uint32_t funct3_bne  = 0x1; 
    static const uint32_t funct3_blt  = 0x4; 
    static const uint32_t funct3_bge  = 0x5; 
    static const uint32_t funct3_bltu = 0x6; 
    static const uint32_t funct3_bgeu = 0x7;


    //funct3 values load
    static const uint32_t funct3_lb  = 0x0; 
    static const uint32_t funct3_lh  = 0x1; 
     static const uint32_t funct3_lw  = 0x2; 
     static const uint32_t funct3_lbu = 0x4; 
    static const uint32_t funct3_lhu = 0x5; 

    //funct3 values store
    static const uint32_t funct3_sb = 0x0; 
     static const uint32_t funct3_sh = 0x1; 
    static const uint32_t funct3_sw = 0x2; 

    //funct3 values ALU imm
    static const uint32_t funct3_addi  = 0x0; 
    static const uint32_t funct3_slli  = 0x1; 
    static const uint32_t funct3_slti  = 0x2; 
     static const uint32_t funct3_sltiu = 0x3; 
    static const uint32_t funct3_xori  = 0x4; 
    static const uint32_t funct3_srli_srai = 0x5; 
     static const uint32_t funct3_ori   = 0x6; 
    static const uint32_t funct3_andi  = 0x7; 

    //funct3 values ALU reg
    static const uint32_t funct3_add_sub = 0x0;
     static const uint32_t funct3_sll     = 0x1; 
    static const uint32_t funct3_slt     = 0x2; 
    static const uint32_t funct3_sltu    = 0x3; 
     static const uint32_t funct3_xor     = 0x4; 
    static const uint32_t funct3_srl_sra = 0x5; 
    static const uint32_t funct3_or      = 0x6; 
    static const uint32_t funct3_and     = 0x7;


    //funct3 values CSR
    static const uint32_t funct3_csrrw = 0x1;
    static const uint32_t funct3_csrrs = 0x2;
    static const uint32_t funct3_csrrc = 0x3;
    static const uint32_t funct3_csrrwi = 0x5;
    static const uint32_t funct3_csrrsi = 0x6;
    static const uint32_t funct3_csrrci = 0x7;

    // System instruction imm fields
    static const uint32_t ecall_imm   = 0x000;
    static const uint32_t ebreak_imm  = 0x001;
    
    //funct7 values ALU reg
    static const uint32_t funct7_add  = 0x00; 
     static const uint32_t funct7_sub  = 0x20; 
    static const uint32_t funct7_sll  = 0x00; 
     static const uint32_t funct7_slt  = 0x00; 
    static const uint32_t funct7_sltu = 0x00; 
     static const uint32_t funct7_xor  = 0x00; 
    static const uint32_t funct7_srl  = 0x00; 
    static const uint32_t funct7_sra  = 0x20; 
    static const uint32_t funct7_or   = 0x00; 
    static const uint32_t funct7_and  = 0x00; 
    static uint32_t get_opcode(uint32_t insn);
    //Decode function
    static std::string decode(uint32_t addr, uint32_t insn);
    static uint32_t get_rd(uint32_t insn);
    static uint32_t get_rs1(uint32_t insn);
    static uint32_t get_rs2(uint32_t insn);
    static uint32_t get_funct3(uint32_t insn);
    static uint32_t get_funct7(uint32_t insn);
    static int32_t get_imm_i(uint32_t insn);
     static int32_t get_imm_u(uint32_t insn);
    static int32_t get_imm_b(uint32_t insn);
    static int32_t get_imm_s(uint32_t insn);
     static int32_t get_imm_j(uint32_t insn);

    //Render functions
    static std::string render_illegal_insn();
    static std::string render_lui(uint32_t insn);
    static std::string render_auipc(uint32_t insn);
     static std::string render_jal(uint32_t addr, uint32_t insn);
    static std::string render_jalr(uint32_t insn);
    static std::string render_btype(uint32_t addr, uint32_t insn, const std::string &mnemonic);
     static std::string render_itype_load(uint32_t insn, const std::string &mnemonic);
    static std::string render_stype(uint32_t insn, const std::string &mnemonic);
    static std::string render_itype_alu(uint32_t insn, const std::string &mnemonic, int32_t imm_i);
    static std::string render_rtype(uint32_t insn, const std::string &mnemonic);

    

    static std::string render_reg(int r);
    static std::string render_base_disp(int32_t imm, int r);
    static std::string render_mnemonic(const std::string &mnemonic);
    static std::string render_csrrx(uint32_t insn, const std::string &mnemonic);
    static std::string render_csrrxi(uint32_t insn, const std::string &mnemonic);
private:
    //Helper functions 
    
     

};

#endif 
