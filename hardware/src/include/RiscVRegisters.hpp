/**
 * MA-Memphis
 * @file RiscVRegisters.hpp
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2019
 * 
 * @brief This file describes the registers and address formats for the 
 * 32-bit XLEN Risc-V.
 */

#pragma once

#include <systemc>
#include <cmath>
#include "definitions.h"

static const uint32_t XLEN = 32;
typedef sc_dt::sc_uint<XLEN> xlenreg_t;

class Privilege {
public:
	enum class Level : uint8_t {
		USER,
		SUPERVISOR,
		MACHINE = 3
	};

private:
	Level reg;

public:
	void set(Level level) { reg = level; }
	Level get() { return reg; };
};

class Register {
protected:
	xlenreg_t reg;
public:
	Register() { reg = 0; }
	Register(xlenreg_t reg_) : reg(reg_) { }
	Register(const Register &Reg) : reg(Reg.reg) { }
	void write(xlenreg_t value) { reg = value; }
	xlenreg_t read() { return reg; }
	sc_dt::sc_uint_subref range(int l, int r) { return reg.range(l, r); }
	sc_dt::sc_uint_bitref bit(int b) { return reg.bit(b); }
};

namespace ISA {
	enum Ext : uint32_t {
		A = 1 << 0,
		B = 1 << 1,
		C = 1 << 2,
		D = 1 << 3,
		E = 1 << 4,
		F = 1 << 5,
		G = 1 << 6,
		H = 1 << 7,
		I = 1 << 8,
		J = 1 << 9,
		K = 1 << 10,
		L = 1 << 11,
		M = 1 << 12,
		N = 1 << 13,
		P = 1 << 15,
		Q = 1 << 16,
		S = 1 << 18,
		T = 1 << 19,
		U = 1 << 20,
		V = 1 << 21,
		X = 1 << 22
	};

	class Misa : public Register {
	public:
		sc_dt::sc_uint_bitref X() { return reg.bit(22); }
		sc_dt::sc_uint_bitref V() { return reg.bit(21); }
		sc_dt::sc_uint_bitref U() { return reg.bit(20); }
		sc_dt::sc_uint_bitref T() { return reg.bit(19); }
		sc_dt::sc_uint_bitref S() { return reg.bit(18); }
		sc_dt::sc_uint_bitref Q() { return reg.bit(16); }
		sc_dt::sc_uint_bitref P() { return reg.bit(15); }
		sc_dt::sc_uint_bitref N() { return reg.bit(13); }
		sc_dt::sc_uint_bitref M() { return reg.bit(12); }
		sc_dt::sc_uint_bitref L() { return reg.bit(11); }
		sc_dt::sc_uint_bitref K() { return reg.bit(10); }
		sc_dt::sc_uint_bitref J() { return reg.bit(9); }
		sc_dt::sc_uint_bitref I() { return reg.bit(8); }
		sc_dt::sc_uint_bitref H() { return reg.bit(7); }
		sc_dt::sc_uint_bitref G() { return reg.bit(6); }
		sc_dt::sc_uint_bitref F() { return reg.bit(5); }
		sc_dt::sc_uint_bitref E() { return reg.bit(4); }
		sc_dt::sc_uint_bitref D() { return reg.bit(3); }
		sc_dt::sc_uint_bitref C() { return reg.bit(2); }
		sc_dt::sc_uint_bitref B() { return reg.bit(1); }
		sc_dt::sc_uint_bitref A() { return reg.bit(0); }
	};
};

namespace Instructions {
	enum class OPCODES {
		OP_IMM   = 0b0010011,
		LUI      = 0b0110111,
		AUIPC    = 0b0010111,
		OP       = 0b0110011,
		JAL      = 0b1101111,
		JALR     = 0b1100111,
		BRANCH   = 0b1100011,
		LOAD     = 0b0000011,
		STORE    = 0b0100011,
		MISC_MEM = 0b0001111,
		SYSTEM   = 0b1110011
	};
	enum class FUNCT3 {
		// OP-IMM
		ADDI  = 0b000,
		SLTI  = 0b010,
		SLTIU = 0b011,
		XORI  = 0b100,
		ORI   = 0b110,
		ANDI  = 0b111,
		SLLI  = 0b001,
		SRLI_SRAI = 0b101,

		// OP
		ADD    = 0b000,
		SUB    = 0b000,
		SLL    = 0b001,
		SLT    = 0b010,
		SLTU   = 0b011,
		XOR    = 0b100,
		SRL	   = 0b101,
		SRA    = 0b101,
		OR     = 0b110,
		AND    = 0b111,
		MUL    = 0b000,
		MULH   = 0b001,
		MULHSU = 0b010,
		MULHU  = 0b011,
		DIV	   = 0b100,
		DIVU   = 0b101,
		REM    = 0b110,
		REMU   = 0b111,

		// JALR
		JALR = 0b000,

		// BRANCH
		BEQ  = 0b000,
		BNE  = 0b001,
		BLT  = 0b100,
		BGE  = 0b101,
		BLTU = 0b110,
		BGEU = 0b111,

		// LOAD
		LB  = 0b000,
		LH  = 0b001,
		LW  = 0b010,
		LBU = 0b100,
		LHU = 0b101,

		// STORE
		SB = 0b000,
		SH = 0b001,
		SW = 0b010,

		// MISC_MEM
		FENCE = 0b000,

		// SYSTEM
		PRIV   = 0b000,
		CSRRW  = 0b001,
		CSRRS  = 0b010,
		CSRRC  = 0b011,
		CSRRWI = 0b101,
		CSRRSI = 0b110,
		CSRRCI = 0b111
	};
	enum class FUNCT7 {
		// OP-IMM
		SLLI = 0b0000000,
		SRLI = 0b0000000,
		SRAI = 0b0100000,

		// OP
		ADD_SLT_SLTU = 0b0000000,
		AND_OR_XOR   = 0b0000000,
		SLL_SRL      = 0b0000000,
		SUB_SRA      = 0b0100000,
		MULDIV 		 = 0b0000001,

		// SYSTEM
		ECALL_EBREAK = 0b0000000,
		SRET_WFI	 = 0b0001000,
		MRET		 = 0b0011000,
		SFENCE_VMA	 = 0b0001001
	};
	enum class RS2 {
		// SYSTEM
		ECALL  = 0b00000,
		EBREAK = 0b00001,
		RET	   = 0b00010,
		WFI    = 0b00101
	};
	enum class RS1 {
		SYSTEM = 0b00000
	};
	enum class RD {
		SYSTEM = 0b00000
	};
};

class Mstatus : public Register {
private:
	static const uint32_t MASK = 0x807FF9BB;
public:
	void write(xlenreg_t value) { reg = (value & MASK);  }
	xlenreg_t read() { return (reg & MASK); }

	sc_dt::sc_uint_bitref SD() { return reg.bit(31); }
	sc_dt::sc_uint_bitref TSR() { return reg.bit(22); }
	sc_dt::sc_uint_bitref TW() { return reg.bit(21); }
	sc_dt::sc_uint_bitref TVM() { return reg.bit(20); }
	sc_dt::sc_uint_bitref MXR() { return reg.bit(19); }
	sc_dt::sc_uint_bitref SUM() { return reg.bit(18); }
	sc_dt::sc_uint_bitref MPRV() { return reg.bit(17); }
	sc_dt::sc_uint_subref XS() { return reg.range(16, 15); }
	sc_dt::sc_uint_subref FS() { return reg.range(14, 13); }
	sc_dt::sc_uint_subref MPP() { return reg.range(12, 11); }
	sc_dt::sc_uint_bitref SPP() { return reg.bit(8); }
	sc_dt::sc_uint_bitref MPIE() { return reg.bit(7); }
	sc_dt::sc_uint_bitref SPIE() { return reg.bit(5); }
	sc_dt::sc_uint_bitref UPIE() { return reg.bit(4); }
	sc_dt::sc_uint_bitref MIE() { return reg.bit(3); }
	sc_dt::sc_uint_bitref SIE() { return reg.bit(1); }
	sc_dt::sc_uint_bitref UIE() { return reg.bit(0); }
};

// @todo Misa. Hard because has some WARL

class Mcause : public Register {
public:
	sc_dt::sc_uint_bitref interrupt() { return reg.bit(31); }
	sc_dt::sc_uint_subref exception_code() { return reg.range(30, 0); }
};

namespace Interrupts {
	enum class CODE {
		USI,
		SSI,
		MSI = 3,

		UTI,
		STI,
		MTI = 7,

		UEI,
		SEI,
		MEI = 11
	};
	enum class BIT {
		USI = 1 << (uint32_t)CODE::USI,
		SSI = 1 << (uint32_t)CODE::SSI,
		MSI = 1 << (uint32_t)CODE::MSI,

		UTI = 1 << (uint32_t)CODE::UTI,
		STI = 1 << (uint32_t)CODE::STI,
		MTI = 1 << (uint32_t)CODE::MTI,

		UEI = 1 << (uint32_t)CODE::UEI,
		SEI = 1 << (uint32_t)CODE::SEI,
		MEI = 1 << (uint32_t)CODE::MEI
	};
	enum class MODE {
		USER = (uint32_t)BIT::USI | (uint32_t)BIT::UTI | (uint32_t)BIT::UEI,
		SUPERVISOR = (uint32_t)BIT::SSI | (uint32_t)BIT::STI | (uint32_t)BIT::SEI,
		MACHINE = (uint32_t)BIT::MSI | (uint32_t)BIT::MTI | (uint32_t)BIT::MEI
	};

	class Mir : public Register {
	private:
		static const uint32_t MASK = 0xAAA;
	public:
		void write(xlenreg_t value) { reg = (value & MASK);  }
		xlenreg_t read() { return (reg & MASK); }

		sc_dt::sc_uint_bitref MEI() { return reg.bit(11); }
		sc_dt::sc_uint_bitref SEI() { return reg.bit(9); }
		//sc_dt::sc_uint_bitref UEI() { return reg.bit(8); }
		sc_dt::sc_uint_bitref MTI() { return reg.bit(7); }
		sc_dt::sc_uint_bitref STI() { return reg.bit(5); }
		//sc_dt::sc_uint_bitref UTI() { return reg.bit(4); }
		sc_dt::sc_uint_bitref MSI() { return reg.bit(3); }
		sc_dt::sc_uint_bitref SSI() { return reg.bit(1); }
		//sc_dt::sc_uint_bitref USI() { return reg.bit(0); }
	};
};

namespace Exceptions {
	enum CODE {
		INSTRUCTION_ADDRESS_MISALIGNED,
		INSTRUCTION_ACCESS_FAULT,
		ILLEGAL_INSTRUCTION,
		BREAKPOINT,
		LOAD_ADDRESS_MISALIGNED,
		LOAD_ACCESS_FAULT,
		STORE_AMO_ADDRESS_MISALIGNED,
		STORE_AMO_ACCESS_FAULT,
		ECALL_FROM_UMODE,
		ECALL_FROM_SMODE,

		ECALL_FROM_MMODE = 11,
		INSTRUCTION_PAGE_FAULT,
		LOAD_PAGE_FAULT,

		STORE_AMO_PAGE_FAULT = 15,
		MAX
	};
	enum BIT {
		IAM = 1 << (uint32_t)CODE::INSTRUCTION_ADDRESS_MISALIGNED,
		IAF = 1 << (uint32_t)CODE::INSTRUCTION_ACCESS_FAULT,
		II  = 1 << (uint32_t)CODE::ILLEGAL_INSTRUCTION,
		BP  = 1 << (uint32_t)CODE::BREAKPOINT,
		LAM = 1 << (uint32_t)CODE::LOAD_ADDRESS_MISALIGNED,
		LAF = 1 << (uint32_t)CODE::LOAD_ACCESS_FAULT,
		SAM = 1 << (uint32_t)CODE::STORE_AMO_ADDRESS_MISALIGNED,
		SAF = 1 << (uint32_t)CODE::STORE_AMO_ACCESS_FAULT,
		ECU = 1 << (uint32_t)CODE::ECALL_FROM_UMODE,
		ECS = 1 << (uint32_t)CODE::ECALL_FROM_SMODE,
		ECM = 1 << (uint32_t)CODE::ECALL_FROM_MMODE,
		IPF = 1 << (uint32_t)CODE::INSTRUCTION_PAGE_FAULT,
		LPF = 1 << (uint32_t)CODE::LOAD_PAGE_FAULT,
		SPF = 1 << (uint32_t)CODE::STORE_AMO_PAGE_FAULT
	};

	class Mer : public Register {
	private:
		static const uint32_t MASK = 0xBFFF;
	public:
		void write(xlenreg_t value) { reg = (value & MASK);  }
		xlenreg_t read() { return (reg & MASK); }

		sc_dt::sc_uint_bitref SPF() { return reg.bit(14); }
		sc_dt::sc_uint_bitref LPF() { return reg.bit(13); }
		sc_dt::sc_uint_bitref IPF() { return reg.bit(12); }
		//sc_dt::sc_uint_bitref ECM() { return reg.bit(11); }
		sc_dt::sc_uint_bitref ECS() { return reg.bit(9); }
		sc_dt::sc_uint_bitref ECU() { return reg.bit(8); }
		sc_dt::sc_uint_bitref SAF() { return reg.bit(7); }
		sc_dt::sc_uint_bitref SAM() { return reg.bit(6); }
		sc_dt::sc_uint_bitref LAF() { return reg.bit(5); }
		sc_dt::sc_uint_bitref LAM() { return reg.bit(4); }
		sc_dt::sc_uint_bitref BP() { return reg.bit(3); }
		sc_dt::sc_uint_bitref II() { return reg.bit(2); }
		sc_dt::sc_uint_bitref IAF() { return reg.bit(1); }
		sc_dt::sc_uint_bitref IAM() { return reg.bit(0); }
	};
};

/* RISC-V instruction format */
class Instruction : public Register {
public:
	sc_dt::sc_uint_subref opcode() { return reg.range(6, 0); }
	sc_dt::sc_uint_subref rd() { return reg.range(11, 7); }
	sc_dt::sc_uint_subref funct3() { return reg.range(14, 12); }
	sc_dt::sc_uint_subref rs1() { return reg.range(19, 15); }
	sc_dt::sc_uint_subref rs2() { return reg.range(24, 20); }
	sc_dt::sc_uint_subref funct7() { return reg.range(31, 25); }
	sc_dt::sc_uint_subref imm_11_0() { return reg.range(31, 20); }
	sc_dt::sc_uint_subref imm_4_0() { return reg.range(11, 7); }
	sc_dt::sc_uint_subref imm_11_5() { return reg.range(31, 25); }
	sc_dt::sc_uint_bitref imm_11_B() { return reg.bit(7); }
	sc_dt::sc_uint_subref imm_4_1() { return reg.range(11, 8); }
	sc_dt::sc_uint_subref imm_10_5() { return reg.range(30, 25); }
	sc_dt::sc_uint_bitref imm_12() { return reg.bit(31); }
	sc_dt::sc_uint_subref imm_31_12() { return reg.range(31, 12); }
	sc_dt::sc_uint_subref imm_19_12() { return reg.range(19, 12); }
	sc_dt::sc_uint_bitref imm_11_J() { return reg.bit(20); }
	sc_dt::sc_uint_subref imm_10_1() { return reg.range(30, 21); }
	sc_dt::sc_uint_bitref imm_20() { return reg.bit(31); }
};

class Address : public Register {
public:
	void next() { reg += 4; }
	sc_dt::sc_uint_subref page_offset() { return reg.range(11, 0); }
};

class Mtvec : public Register {
public:
	enum class Mode {
		DIRECT,
		VECTORED
	};
	sc_dt::sc_uint_subref BASE() { return reg.range(31, 2); }
	sc_dt::sc_uint_subref MODE() { return reg.range(1, 0); }
};

namespace Sv32 {
	const uint32_t PAGESIZE = 4096;
	const uint32_t LEVELS = 2;
	const uint32_t PTESIZE = 4;

	class VirtualAddress : public Address {
	public:
		VirtualAddress(Address addr) { reg = addr.read(); }
		sc_dt::sc_uint_subref VPN(int level) { return level ? reg.range(31, 22) : reg.range(21, 12); }
	};

	class PhysicalAddress : public Address {
	private:
		sc_dt::sc_uint<34> reg;
	public:
		PhysicalAddress();
		PhysicalAddress(sc_dt::sc_uint<34> pa) : reg(pa) {};
		sc_dt::sc_uint_subref PPN(int level) { return level ? reg.range(33, 22) : reg.range(21, 12); }
		sc_dt::sc_uint_subref PPN() { return reg.range(33, 12); }
	};

	class PageTableEntry : public Register {
	public:
		sc_dt::sc_uint_subref PPN(int level) { return level ? reg.range(31, 20) : reg.range(19, 10); }
		sc_dt::sc_uint_subref PPN() { return reg.range(31, 10); }

		//sc_dt::sc_uint_subref RSW() { return reg.range(9, 8); }
		sc_dt::sc_uint_bitref D() { return reg.bit(7); }
		sc_dt::sc_uint_bitref A() { return reg.bit(6); }
		sc_dt::sc_uint_bitref G() { return reg.bit(5); }
		sc_dt::sc_uint_bitref U() { return reg.bit(4); }
		sc_dt::sc_uint_bitref X() { return reg.bit(3); }
		sc_dt::sc_uint_bitref W() { return reg.bit(2); }
		sc_dt::sc_uint_bitref R() { return reg.bit(1); }
		sc_dt::sc_uint_bitref V() { return reg.bit(0); }
	};
};

class Satp : public Register {
public:
	enum MODES {
		BARE,
		Sv32
	};
	sc_dt::sc_uint_bitref MODE() { return reg.bit(31); }
	sc_dt::sc_uint_subref ASID() { return reg.range(30, 22); }
	sc_dt::sc_uint_subref PPN() { return reg.range(21, 0); }
};

namespace Timings {
	static const uint32_t RESET = 17;
	static const uint32_t MEM_READ = 2;
	static const uint32_t MEM_WRITE = 1;
	static const uint32_t DECODE = 1;
	static const uint32_t LOGICAL = 1;
	static const uint32_t MUL = 1;
	static const uint32_t DIV = 1;
	static const uint32_t CSR = 1;
	static const uint32_t FETCH = 1;
};

namespace CSR
{
	static const uint16_t RWA_MASK = 0x0C00;
	static const uint16_t LVL_MASK = 0x0300;
	static const uint16_t RO = 0x0C00;
	static const uint16_t LVL_SHIFT = 8;

	enum Address : uint16_t {
		/* Supervisor CSRs */
		SSTATUS = 0x100,
		SIE		= 0x104,
		STVEC	= 0x105,
		
		SSCRATCH = 0x140,
		SEPC,
		SCAUSE,
		STVAL,
		SIP,

		SATP	 = 0x180,

		/* Machine CSRs */
		MVENDORID = 0xF11,
		MARCHID,
		MIMPID,
		MHARTID,

		MSTATUS = 0x300,
		MISA,
		MEDELEG,
		MIDELEG,
		MIE,
		MTVEC,
		
		MSCRATCH = 0x340,
		MEPC,
		MCAUSE,
		MTVAL,
		MIP,

		MRAR = 0x7C0
	};
};

class Mrar : public Register {
public:
	enum MODES {
		OFFSET,
		SATP
	};

	sc_dt::sc_uint_bitref MODE() { return reg.bit(0); }
};