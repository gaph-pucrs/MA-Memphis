/**
 * MA-Memphis
 * @file RiscV.hpp
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2019
 * 
 * @brief
 * Header file for the RISC-V RV32I Instruction Set Simulator (ISS)
 */

#pragma once

#include "RiscVRegisters.hpp"

#include <systemc>
#include <stdint.h>
#include "standards.h"
#include "definitions.h"

/* Length of a flit */
static const int FLIT_SIZE = 32;
static const int HALF_FLIT = FLIT_SIZE/2;
typedef sc_uint<HALF_FLIT> half_flit_t;

namespace vectors {
	const uint32_t RESET = 0;
};

SC_MODULE(RiscV) {
public:
	SC_HAS_PROCESS(RiscV);

	/**
	 * Module ports
	 * All we be mantained to keep compatibility with HeMPS framework.
	 */
	sc_in_clk	clk;
	sc_in<bool> reset_in;
	sc_in<bool> intr_in;

	sc_out<sc_uint<32> > mem_address;	// Memory address bus
	sc_in<sc_uint<32> >  mem_data_r;	// Memory data read bus
	sc_out<sc_uint<4> >  mem_byte_we;	// Memory data write byte select
	sc_out<sc_uint<32> > mem_data_w;	// Memory data write bus

	/* Not in use. Declared for compatibility reasons */
	sc_in<bool> mem_pause;

	/* Don't know if in use */
	sc_out<sc_uint<8> > current_page;

	/* Use for statistics */
	unsigned long int pc_count;
	unsigned long int global_inst;
	unsigned long int logical_inst;
	unsigned long int branch_inst;	  
	unsigned long int jump_inst;
	unsigned long int move_inst;
	unsigned long int other_inst;
	unsigned long int arith_inst;
	unsigned long int load_inst;
	unsigned long int shift_inst;
	unsigned long int nop_inst;
	unsigned long int mult_div_inst;
	/* Instructions for PAGE 0 (KERNEL) */
	unsigned long int global_inst_kernel;
	unsigned long int logical_inst_kernel;
	unsigned long int branch_inst_kernel;	  
	unsigned long int jump_inst_kernel;
	unsigned long int move_inst_kernel;
	unsigned long int other_inst_kernel;
	unsigned long int arith_inst_kernel;
	unsigned long int load_inst_kernel;
	unsigned long int shift_inst_kernel;
	unsigned long int nop_inst_kernel;
	unsigned long int mult_div_inst_kernel;	  
	/* Instructions for PAGES different from 0 (TASKS) */
	unsigned long int global_inst_tasks;
	unsigned long int logical_inst_tasks;
	unsigned long int branch_inst_tasks;	  
	unsigned long int jump_inst_tasks;
	unsigned long int move_inst_tasks;
	unsigned long int other_inst_tasks;
	unsigned long int arith_inst_tasks;
	unsigned long int load_inst_tasks;
	unsigned long int shift_inst_tasks;
	unsigned long int nop_inst_tasks;
	unsigned long int mult_div_inst_tasks;

	/**
	 * @brief The loop of the RISC-V CPU.
	 * 
	 * @details Fetch, decode, execute, write-back
	 */
	void cpu();

	/**
	 * @brief SC_MODULE constructor.
	 * 
	 * @param name_ The SC_MODULE name
	 * @param router_addr_ The PE router address.
	 */
	RiscV(sc_module_name name_, half_flit_t router_addr_ = 0);

private:
	static const uint8_t PAGE_SHIFT;

	/* PE router address. Used by the simulator */
	half_flit_t router_addr;

	//@todo Global inst register?

	/* GPRs "X" registers */
	Register x[32];

	/* Program counter */
	Address pc;

	/* This is not a regular register. It just keeps track of the current privilege level */
	Privilege priv;

	/* Fetched instruction register */
	Instruction instr;

	/**
	 * Machine-level CSRs
	 * 
	 * MACHINE INFORMATION REGISTERS
	 * mvendorid:	Vendor ID.
	 * marchid:		Architecture ID.
	 * mimpid:		Implementation ID.
	 * mhartid:		Hardware thread ID.
	 * 
	 * MACHINE TRAP SETUP
	 * mstatus:		Machine status register.
	 * misa:		ISA and extensions.
	 * medeleg:		Machine exception delegation register.
	 * mideleg:		Machine interrupt delegation register.
	 * mie:			Machine interrupt-enable register.
	 * mtvec:		Machine trap-handler base address.
	 * mcounteren:	Machine conter enable.
	 * 
	 * MACHINE TRAP HANDLING
	 * mscratch:	Scratch register for machine trap handlers
	 * mepc:		Machine exception program counter.
	 * mcause:		Machine trap cause.
	 * mtval:		Machine bad address or instruction.
	 * mip:			Machine interrupt pending.
	 */
	Register mvendorid;
	Register marchid;
	Register mimpid;
	Register mhartid;
	Mstatus mstatus;
	ISA::Misa misa;
	Exceptions::Mer medeleg;
	Interrupts::Mir mideleg;
	Interrupts::Mir mie;
	Mtvec mtvec;
	//mcounteren
	Register mscratch;
	Address mepc;
	Mcause mcause;
	Register mtval;
	Interrupts::Mir mip;

	/**
	 * Supervisor-level CSRs
	 * 
	 * SUPERVISOR TRAP SETUP
	 * sstatus:		Supervisor status register.
	 * sedeled:		Supervisor exception delegation register.
	 * sideleg:		Supervisor interrupt delegation register.
	 * sie:			Supervisor interrupt-enable register.
	 * stvec:		Supervisor trap handler base address.
	 * scounteren:	Supervisor counter enable.
	 * 
	 * SUPERVISOR TRAP HANDLING
	 * sscratch:	Supervisor register for supervisor trap handlers.
	 * sepc:		Supervisor exception program counter.
	 * scause:		Supervisor trap cause.
	 * stval:		Supervisor bad address or instruction.
	 * sip:			Supervisor interrupt pending.
	 * 
	 * SUPERVISOR PROTECTION AND TRANSLATION
	 * satp:		Supervisor address translation and protection.
	 * 
	 * SUPERVISOR CUSTOM REGISTERS
	 * srar:		Supervisor relative address register.
	 */
	//sstatus
	//sedeleg
	//sideleg
	//sie
	Mtvec stvec;
	//scounteren
	Register sscratch;
	Address sepc;
	Mcause scause;
	Register stval;
	//sip
	Satp satp;
	Mrar mrar;

	/**
	 * @brief Resets the CPU to its initial state.
	 */
	void reset();

	/**
	 * @brief Handle pending interrupts.
	 * 
	 * @detail If interrupt is not masked, take it.
	 * 
	 * @return True if interrupt has been taken.
	 */
	bool handle_interrupts();

	/**
	 * @brief Fetches an instruction.
	 * 
	 * @return True if exception occurred
	 */
	bool fetch();

	/**
	 * @brief Resolves paging Sv32/Bare mode
	 * 
	 * @param src_addr	Source address to load/store
	 * @param &dst_addr Physical address to load/store
	 * @param e_code	Exception code in case of page fault
	 * 
	 * @return True if exception occurred
	 */
	bool paging(Address src_addr, Address &dst_addr, Exceptions::CODE e_code);

	/**
	 * @brief Reads XLEN from memory.
	 * 
	 * @param address The memory address to be readed.
	 * 
	 * @return XLEN bits from memory address.
	 */
	xlenreg_t mem_read(sc_uint<34> address);

	/**
	 * @brief Writes XLEN to memory.
	 * 
	 * @param address 	The memory address to be written.
	 * @param value		The value to be written
	 * @param byte		The write enable byte to write.
	 */
	void mem_write(sc_uint<34> address, xlenreg_t value, uint8_t byte);

	/**
	 * @brief Handles synchronous exceptions
	 */
	void handle_exceptions(Exceptions::CODE code);

	/**
	 * @brief Decodes and executes the fetched instruction.
	 * 
	 * @return True if exception occurred
	 */
	bool decode();

	/**
	 * @brief Decodes the OP-IMM opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_op_imm();

	/**
	 * @brief Decodes the OP opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_op();

	/**
	 * @brief Decodes the BRANCH opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_branch();

	/**
	 * @brief Decodes the LOAD opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_load();

	/**
	 * @brief Decodes the LOAD opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_store();

	/**
	 * @brief Decodes the LOAD opcode.
	 * 
	 * @return True if exception occurred
	 */
	bool decode_system();

	// RV32I Instructions
	/**
	 * @brief Load Upper Immediate.
	 * 
	 * @detail rd ← imm
	 * 
	 * @return False
	 */
	bool lui();

	/**
	 * @brief Add Upper Immediate to PC.
	 * 
	 * @detail 	rd ← pc + offset
	 * 
	 * @return False
	 */
	bool auipc();

	/**
	 * @brief Jump and Link
	 * 
	 * @detail rd ← pc + length(inst)
	 * 		   pc ← pc + offset
	 * 
	 * @return True. Can generate Instruction Address Misaligned.
	 */
	bool jal();

	/**
	 * @brief Jump and Link Register
	 * 
	 * @detail rd ← pc + length(inst)
	 * 		   pc ← (rs1 + offset) ∧ -2
	 * 
	 * @return True. Can generate Instruction Address Misaligned.
	 */
	bool jalr();

	/**
	 * @brief Branch Equal
	 * 
	 * @detail if rs1 = rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool beq();

	/**
	 * @brief Branch Not Equal
	 * 
	 * @detail if rs1 ≠ rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool bne();

	/**
	 * @brief Branch Less Than
	 * 
	 * @detail if rs1 < rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool blt();

	/**
	 * @brief Branch Greater than Equal
	 * 
	 * @detail if rs1 ≥ rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool bge();

	/**
	 * @brief Branch Less Than Unsigned
	 * 
	 * @detail if rs1 < rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool bltu();

	/**
	 * @brief Branch Greater than Equal Unsigned
	 * 
	 * @detail if rs1 ≥ rs2 then pc ← pc + offset
	 * 
	 * @return False if branch not taken.
	 * 		   True if branch taken. Can generate Instruction Address Misaligned.
	 */
	bool bgeu();

	/**
	 * @brief Load Byte
	 * 
	 * @detail rd ← s8[rs1 + offset]
	 * 
	 * @return True if exception occurred
	 */
	bool lb();

	/**
	 * @brief Load Half
	 * 
	 * @detail rd ← s16[rs1 + offset]
	 * 
	 * @return True if exception occurred.
	 */
	bool lh();

	/**
	 * @brief Load Word
	 * 
	 * @detail rd ← s32[rs1 + offset]
	 * 
	 * @return True if exception occurred.
	 */
	bool lw();

	/**
	 * @brief Load Byte Unsigned
	 * 
	 * @detail rd ← u8[rs1 + offset]
	 * 
	 * @return True if exception occurred
	 */
	bool lbu();

	/**
	 * @brief Load Half Unsigned
	 * 
	 * @detail rd ← u16[rs1 + offset]
	 * 
	 * @return True if exception occurred.
	 */
	bool lhu();

	/**
	 * @brief Store Byte
	 * 
	 * @detail u8[rs1 + offset] ← rs2
	 * 
	 * @return True if exception occurred.
	 */
	bool sb();

	/**
	 * @brief Store Half
	 * 
	 * @detail u16[rs1 + offset] ← rs2
	 * 
	 * @return True if exception occurred.
	 */
	bool sh();

	/**
	 * @brief Store Word
	 * 
	 * @detail u32[rs1 + offset] ← rs2
	 * 
	 * @return True if exception occurred.
	 */
	bool sw();

	/**
	 * @brief Add Immediate
	 * 
	 * @detail rd ← rs1 + sx(imm)
	 * 
	 * @return False.
	 */
	bool addi();

	/**
	 * @brief Set Less Than Immediate
	 * 
	 * @detail rd ← sx(rs1) < sx(imm)
	 * 
	 * @return False.
	 */
	bool slti();

	/**
	 * @brief Set Less Than Immediate Unsigned
	 * 
	 * @detail rd ← ux(rs1) < ux(imm)
	 * 
	 * @return False.
	 */
	bool sltiu();

	/**
	 * @brief Xor Immediate
	 * 
	 * @detail rd ← ux(rs1) ⊕ ux(imm)
	 * 
	 * @return False.
	 */
	bool xori();

	/**
	 * @brief Or Immediate
	 * 
	 * @detail rd ← ux(rs1) ∨ ux(imm)
	 * 
	 * @return False.
	 */
	bool ori();

	/**
	 * @brief And Immediate
	 * 
	 * @detail rd ← ux(rs1) ∧ ux(imm)
	 * 
	 * @return False.
	 */
	bool andi();

	/**
	 * @brief Shift Left Logical Immediate
	 * 
	 * @detail rd ← ux(rs1) ∧ ux(imm)
	 * 
	 * @return False.
	 */
	bool slli();

	/**
	 * @brief Shift Right Logical Immediate
	 * 
	 * @detail rd ← ux(rs1) » ux(imm)
	 * 
	 * @return False.
	 */
	bool srli();

	/**
	 * @brief Shift Right Arithmetic Immediate
	 * 
	 * @detail rd ← sx(rs1) » ux(imm)
	 * 
	 * @return False.
	 */
	bool srai();

	/**
	 * @brief Add
	 * 
	 * @detail rd ← sx(rs1) + sx(rs2)
	 * 
	 * @return False.
	 */
	bool add();

	/**
	 * @brief Subtract
	 * 
	 * @detail rd ← sx(rs1) - sx(rs2)
	 * 
	 * @return False.
	 */
	bool sub();

	/**
	 * @brief Shift Left Logical
	 * 
	 * @detail rd ← ux(rs1) « rs2
	 * 
	 * @return False.
	 */
	bool sll();

	/**
	 * @brief Set Less Than
	 * 
	 * @detail rd ← sx(rs1) < sx(rs2)
	 * 
	 * @return False.
	 */
	bool slt();

	/**
	 * @brief Set Less Than Unsigned
	 * 
	 * @detail rd ← ux(rs1) < ux(rs2)
	 * 
	 * @return False.
	 */
	bool sltu();

	/**
	 * @brief Xor
	 * 
	 * @detail rd ← ux(rs1) ⊕ ux(rs2)
	 * 
	 * @return False.
	 */
	bool _xor();

	/**
	 * @brief Shift Right Logical
	 * 
	 * @detail rd ← ux(rs1) » rs2
	 * 
	 * @return False.
	 */
	bool srl();

	/**
	 * @brief Shift Right Arithmetic
	 * 
	 * @detail rd ← sx(rs1) » rs2
	 * 
	 * @return False.
	 */
	bool sra();

	/**
	 * @brief Or
	 * 
	 * @detail rd ← ux(rs1) ∨ ux(rs2)
	 * 
	 * @return False.
	 */
	bool _or();

	/**
	 * @brief And
	 * 
	 * @detail rd ← ux(rs1) ∧ ux(rs2)
	 * 
	 * @return False.
	 */
	bool _and();

	/**
	 * @brief Fence
	 * 
	 * @detail Implemented as NOP.
	 * 
	 * @return False.
	 */
	bool fence();

	/**
	 * @brief Service Request to the Execution Environment
	 * 
	 * @return True.
	 */
	bool ecall();

	/**
	 * @brief Return Control to a Debugging Environment
	 * 
	 * @detail Implemented as NOP
	 * 
	 * @return False.
	 */
	bool ebreak();

	/**
	 * @brief Multiply
	 * 
	 * @detail rd ← ux(rs1) × ux(rs2)
	 * 
	 * @return False.
	 */
	bool mul();
	
	/**
	 * @brief Multiply High Signed Signed
	 * 
	 * @detail rd ← (sx(rs1) × sx(rs2)) » xlen
	 * 
	 * @return False.
	 */
	bool mulh();
	
	/**
	 * @brief Multiply High Signed Unsigned
	 * 
	 * @detail rd ← (sx(rs1) × ux(rs2)) » xlen
	 * 
	 * @return False.
	 */
	bool mulhsu();
	
	/**
	 * @brief Multiply High Unsigned Unsigned
	 * 
	 * @detail rd ← (ux(rs1) × ux(rs2)) » xlen
	 * 
	 * @return False.
	 */
	bool mulhu();
	
	/**
	 * @brief Divide Signed
	 * 
	 * @detail rd ← sx(rs1) ÷ sx(rs2)
	 * 
	 * @return False.
	 */
	bool div();
	
	/**
	 * @brief Divide Unsigned
	 * 
	 * @detail rd ← ux(rs1) ÷ ux(rs2)
	 * 
	 * @return False.
	 */
	bool divu();
	
	/**
	 * @brief Remainder Signed
	 * 
	 * @detail rd ← sx(rs1) mod sx(rs2)
	 * 
	 * @return False.
	 */
	bool rem();
	
	/**
	 * @brief Remainder Unsigned
	 * 
	 * @detail rd ← ux(rs1) mod ux(rs2)
	 * 
	 * @return False.
	 */
	bool remu();

	/**
	 * @brief Atomic Read/Write CSR
	 * 
	 * @return False.
	 */
	bool csrrw();

	/**
	 * @brief Atomic Read and Set Bits in CSR
	 * 
	 * @return True if exception raised.
	 */
	bool csrrs();

	/**
	 * @brief Atomic Read and Clear Bits in CSR
	 * 
	 * @return True if exception raised.
	 */
	bool csrrc();

	/**
	 * @brief Atomic Read/Write CSR Immediate
	 * 
	 * @return True if exception raised.
	 */
	bool csrrwi();

	/**
	 * @brief Atomic Read and Set Bits in CSR Immediate
	 * 
	 * @return True if exception raised.
	 */
	bool csrrsi();

	/**
	 * @brief Atomic Read and Clear Bits in CSR Immediate
	 * 
	 * @return True if exception raised.
	 */
	bool csrrci();

	/**
	 * @brief Return from trap under S-Mode
	 * 
	 * @return True. Can raise exception.
	 */
	bool sret();

	/**
	 * @brief Return from trap under M-Mode
	 * 
	 * @return True. Can raise exception.
	 */
	bool mret();


	/**
	 * @brief Wait for Interrupt
	 * 
	 * @detail Implemented as NOP but can raise exception.
	 * 
	 * @return True if exception raised.
	 */
	bool wfi();

	/**
	 * @brief Supervisor memory-management fence
	 * 
	 * @detail Implemented as NOP.
	 * 
	 * @return True if exception raised.
	 */
	bool sfence_vma();

	/**
	 * @brief Pointer to execute function that will be set by the decoder.
	 * 
	 * @return True if jump happened by branch, jump or exception.
	 */
	bool (RiscV::*execute)();
	

	/**
	 * @brief "Decodes" CSR and check permissions.
	 * 
	 * @param addr			Address of the CSR sent by the instruction.
	 * @param rw			If instruction is RW.
	 * @param *csr			Pointer to store the decoded CSR.
	 * @param &wmask_and	AND mask for the CSR write.
	 * @param &wmask_or		OR mask for the CSR write.
	 * @param &rmask		AND mask for the CSR read.
	 * 
	 * @return True if invalid CSR of permission, raising exception.
	 */
	bool csr_helper(uint16_t addr, bool rw, Register* &csr, uint32_t &wmask_and, uint32_t &wmask_or, uint32_t &rmask);
};