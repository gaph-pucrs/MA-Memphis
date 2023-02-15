/**
 * MA-Memphis
 * @file RiscV.cpp
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2019
 * 
 * @brief
 * Source file for a generic RISC-V CPU ISS running a RV32I ISA
 * with M/S/U privileges.
 */

#include "RiscV.hpp"

#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(RiscV);
#endif

const uint8_t RiscV::PAGE_SHIFT = (unsigned char)(log2(PAGE_SIZE_BYTES));

RiscV::RiscV(sc_module_name name_, half_flit_t router_addr_) : 
				sc_module(name_), router_addr(router_addr_),
				mvendorid(0), marchid(0), mimpid(0), mhartid(0)
{
	pc_count				= 0;
	logical_inst			= 0;
	jump_inst				= 0;
	branch_inst				= 0;
	move_inst				= 0;
	other_inst				= 0;
	arith_inst				= 0;
	load_inst				= 0;
	shift_inst				= 0;	
	nop_inst				= 0;	
	mult_div_inst			= 0;
	/* Instructions for PAGE 0 (KERNEL) */
	global_inst_kernel		= 0;
	logical_inst_kernel		= 0;
	branch_inst_kernel		= 0;	
	jump_inst_kernel		= 0;
	move_inst_kernel		= 0;
	other_inst_kernel		= 0;
	arith_inst_kernel		= 0;
	load_inst_kernel		= 0;
	shift_inst_kernel		= 0;	
	nop_inst_kernel			= 0;	
	mult_div_inst_kernel	= 0;
	/* Instructions for PAGES different from 0 (TASKS) */
	global_inst_tasks		= 0;
	logical_inst_tasks		= 0;
	jump_inst_tasks			= 0;
	branch_inst_tasks		= 0;
	move_inst_tasks			= 0;
	other_inst_tasks		= 0;
	arith_inst_tasks		= 0;
	load_inst_tasks			= 0;
	shift_inst_tasks		= 0;	
	nop_inst_tasks			= 0;	
	mult_div_inst_tasks		= 0;

	SC_THREAD(cpu);
	sensitive << clk.pos();// << mem_pause.pos();
	//sensitive << mem_pause.neg();
}

void RiscV::cpu()
{
	reset();

	while(true) {
		// Don't save PC on mem_pause: deprecated
		// @todo Global inst CSR?

		/* Stats */
		global_inst_kernel	= logical_inst_kernel + branch_inst_kernel + jump_inst_kernel + move_inst_kernel + other_inst_kernel + arith_inst_kernel + load_inst_kernel + shift_inst_kernel + nop_inst_kernel + mult_div_inst_kernel;
		global_inst_tasks	= logical_inst_tasks + branch_inst_tasks + jump_inst_tasks + move_inst_tasks + other_inst_tasks + arith_inst_tasks + load_inst_tasks + shift_inst_tasks + nop_inst_tasks + mult_div_inst_tasks;
		logical_inst		= logical_inst_kernel + logical_inst_tasks;
		branch_inst			= branch_inst_kernel + branch_inst_tasks;
		jump_inst			= jump_inst_kernel + jump_inst_tasks;
		move_inst			= move_inst_kernel + move_inst_tasks;
		other_inst			= other_inst_kernel + other_inst_tasks;
		arith_inst			= arith_inst_kernel + arith_inst_tasks;
		load_inst			= load_inst_kernel + load_inst_tasks;
		shift_inst			= shift_inst_kernel + shift_inst_tasks;			
		nop_inst			= nop_inst_kernel + nop_inst_tasks;			
		mult_div_inst		= mult_div_inst_kernel + mult_div_inst_tasks;
		global_inst			= global_inst_kernel + global_inst_tasks;

		x[0].write(0);

		if(reset_in.read()){
			reset();
			continue;
		}

		mip.MEI() = intr_in.read();
		if(handle_interrupts())	// If interrupt is handled, continues interrupt PC
			continue;

		if(fetch())	// If exception occurred, continues to exception PC
			continue;

		if(decode()) // If exception occurred, continues to exception PC
			continue;

		if((this->*execute)())	// If branch or exception, continues to
			continue;			// exception/branch/jump address

		pc.next();	// If normal execution go to next PC
	}

}

void RiscV::reset()
{
	priv.set(Privilege::Level::MACHINE);
	mstatus.MIE() = 0;
	mstatus.MPRV() = 0;
	misa.write((ISA::Ext::M | ISA::Ext::S | ISA::Ext::U));
	pc.write(vectors::RESET);
	mcause.write(0);

	// Stat
	global_inst = 0;

	mem_byte_we.write(0x0);
	wait(Timings::RESET);
}

bool RiscV::handle_interrupts()
{
	// Machine-level interrupt. Can only be masked by M-Mode
	if((mip.read() & mie.read() & ~mideleg.read()) && 
					(priv.get() != Privilege::Level::MACHINE || mstatus.MIE())){
		if(mip.MEI() && mie.MEI() && !mideleg.MEI()) // Machine External Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::MEI;
		else if(mip.MSI() && mie.MSI() && !mideleg.MSI()) // Machine Software Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::MSI;
		else if(mip.MTI() && mie.MTI() && !mideleg.MTI()) // Machine Timer Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::MTI;
		else if(mip.SEI() && mie.SEI() && !mideleg.SEI()) // Supervisor External Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::SEI;
		else if(mip.SSI() && mie.SSI() && !mideleg.SSI()) // Supervisor Software Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::SSI;
		else if(mip.STI() && mie.STI() && !mideleg.STI()) // Supervisor Timer Interrupt
			mcause.exception_code() = (uint32_t)Interrupts::CODE::STI;
		
		mcause.interrupt() = 1;
		mtval.write(0);
		mstatus.MPP() = (uint32_t)priv.get(); // Previous privilege
		priv.set(Privilege::Level::MACHINE);  // New privilege
		mstatus.MPIE() = mstatus.MIE();		  // Previous interrupt-enable of target mode
		mstatus.MIE() = 0;					  // Disable interrupt-enable of target mode
		mepc.write(pc.read());				  // Previous PC
		if(mtvec.MODE() == (uint32_t)Mtvec::Mode::VECTORED){ // New pc
			pc.write((mtvec.BASE() << 2) + 4 * mcause.exception_code());
		} else { // Direct
			pc.write((mtvec.BASE() << 2));
		}
		return true;	// Interrupt taken
	} else if((mip.read() & mie.read() & mideleg.read()) && // Supervisor-level interrupt
				(priv.get() == Privilege::Level::USER || 	// U-Mode can't mask
				(priv.get() == Privilege::Level::SUPERVISOR && mstatus.SIE()))){ // S-Mode interrupt enabled
		if(mip.MEI() && mie.MEI() && mideleg.MEI()) // Machine External Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::MEI;
		else if(mip.MSI() && mie.MSI() && mideleg.MSI()) // Machine Software Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::MSI;
		else if(mip.MTI() && mie.MTI() && mideleg.MTI()) // Machine Timer Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::MTI;
		else if(mip.SEI() && mie.SEI() && mideleg.SEI()) // Supervisor External Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::SEI;
		else if(mip.SSI() && mie.SSI() && mideleg.SSI()) // Supervisor Software Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::SSI;
		else if(mip.STI() && mie.STI() && mideleg.STI()) // Supervisor Timer Interrupt
			scause.exception_code() = (uint32_t)Interrupts::CODE::STI;
		
		scause.interrupt() = 1;
		stval.write(0);
		mstatus.SPP() = (uint32_t)priv.get();	// Previous privilege
		priv.set(Privilege::Level::SUPERVISOR);	// New privilege
		mstatus.SPIE() = mstatus.SIE();			// Previous interrupt-enable of target mode
		mstatus.SIE() = 0;						// Disable interrupt-enable of target mode
		sepc.write(pc.read());					// Previous PC
		if(stvec.MODE() == (uint32_t)Mtvec::Mode::VECTORED){ // New pc
			pc.write((stvec.BASE() << 2) + 4 * scause.exception_code());
		} else { // Direct
			pc.write((stvec.BASE() << 2));
		}
		return true;	// Interrupt taken
	} // User-level interrupts not implemented
	return false;	// Interrupt not taken
}

bool RiscV::fetch()
{
	// wait(Timings::FETCH);
	Address phy_pc;
	if(paging(pc, phy_pc, Exceptions::CODE::INSTRUCTION_PAGE_FAULT))
		return true;
	
	instr.write(mem_read(phy_pc.read()));

	// stat
	pc_count = phy_pc.read();

	return false;
}

bool RiscV::paging(Address src_addr, Address &dst_addr, Exceptions::CODE e_code)
{
	if(priv.get() == Privilege::Level::MACHINE ||	// M-Mode is bare mode
		(satp.MODE() == Satp::MODES::BARE && mrar.MODE() == Mrar::MODES::SATP)){
		current_page.write(0);
		dst_addr.write(src_addr.read());
		return false;
	} else if(mrar.MODE() == Mrar::MODES::OFFSET){		
		if(priv.get() == Privilege::Level::SUPERVISOR){ // Kernel is page 0
			// @todo Inform externally of current page ??
			current_page.write(0);
			dst_addr.write(src_addr.read());
			return false;
		} else { // U-Mode with offset
			current_page.write(mrar.read()>>PAGE_SHIFT);
			dst_addr.write(src_addr.read() | mrar.read());
			return false;
		}
	} else { // Sv32
		Sv32::VirtualAddress va(src_addr);
		Sv32::PhysicalAddress a(satp.PPN() * Sv32::PAGESIZE);
		for(int i = Sv32::LEVELS - 1; i >= 0; i--){
			Sv32::PhysicalAddress pte_addr(a.read() + va.VPN(i)*Sv32::PTESIZE);
			Sv32::PageTableEntry pte;
			pte.write(mem_read(pte_addr.read()));
			if(!pte.V() || (!pte.R() && pte.W())){
				// Not valid or Write-Only
				handle_exceptions(e_code);
				return true;
			} else if(pte.R() || pte.X()){
				// Leaf PTE
				if(pte.X() && 
					((priv.get() == Privilege::Level::USER && pte.U()) || 
					 (priv.get() == Privilege::Level::SUPERVISOR && !pte.U()))){
					// Memory execute allowed
					if(i && pte.PPN(0)){
						// Misaligned superpage
						handle_exceptions(e_code);
						return true;
					} else {
						// Efectively access PTE
						pte.A() = 1;
						Sv32::PhysicalAddress pa(0);
						pa.page_offset() = va.page_offset();
						if(i){
							// Superpage translation
							pa.PPN(0) = va.VPN(0);
						}
						pa.PPN(Sv32::LEVELS - 1) = pte.PPN(Sv32::LEVELS - 1);
						dst_addr.write(pa.PPN()*Sv32::PAGESIZE + pa.page_offset());
						return false;
					}
				} else {
					// Invalid access type
					handle_exceptions(e_code);
					return true;
				}
			} else {
				// Pointer to next-level PTE
				a.write(pte.PPN() * Sv32::PAGESIZE);
				continue;
			}
		}
		handle_exceptions(e_code);
		return true;
	}
}

xlenreg_t RiscV::mem_read(sc_uint<34> address)
{
	mem_address.write(address);
	wait(Timings::MEM_READ);
	xlenreg_t ret = mem_data_r.read();
	// xlenreg_t ret;
	// ret.range(31, 24) = val.range(7, 0);
	// ret.range(23, 16) = val.range(15, 8);
	// ret.range(15, 8) = val.range(23, 16);
	// ret.range(7, 0) = val.range(31, 24);
	return ret;
}

void RiscV::mem_write(sc_uint<34> address, xlenreg_t value, uint8_t byte)
{
	// xlenreg_t arg;
	// arg.range(31, 24) = value.range(7, 0);
	// arg.range(23, 16) = value.range(15, 8);
	// arg.range(15, 8) = value.range(23, 16);
	// arg.range(7, 0) = value.range(31, 24);

	mem_address.write(address);
	mem_data_w.write(value);
	mem_byte_we.write(byte);	// Enable write
	wait(Timings::MEM_WRITE);
	mem_byte_we.write(0);		// Disable write
	// wait(Timings::MEM_WRITE);
}

void RiscV::handle_exceptions(Exceptions::CODE code)
{
	if(priv.get() != Privilege::Level::MACHINE && (medeleg.read() & (1 << code))){ // Handle in S-Mode
		scause.interrupt() = 0;
		scause.exception_code() = code;
		mstatus.SPP() = (uint32_t)priv.get();	// Previous privilege
		priv.set(Privilege::Level::SUPERVISOR);	// New privilege
		mstatus.SPIE() = mstatus.SIE();			// Previous interrupt-enable of target mode
		mstatus.SIE() = 0;						// Disable interrupt-enable of target mode
		sepc.write(pc.read()+4);				// Previous PC
		switch(code){
			case Exceptions::CODE::INSTRUCTION_ACCESS_FAULT:
			case Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED:
			case Exceptions::CODE::INSTRUCTION_PAGE_FAULT:
			case Exceptions::CODE::LOAD_ACCESS_FAULT:
			case Exceptions::CODE::LOAD_ADDRESS_MISALIGNED:
			case Exceptions::CODE::LOAD_PAGE_FAULT:
			case Exceptions::CODE::STORE_AMO_ACCESS_FAULT:
			case Exceptions::CODE::STORE_AMO_ADDRESS_MISALIGNED:
			case Exceptions::CODE::STORE_AMO_PAGE_FAULT:
			case Exceptions::CODE::BREAKPOINT:
				stval.write(pc.read());
				break;
			case Exceptions::ILLEGAL_INSTRUCTION:
				stval.write(instr.read());
				break;
			default:
				stval.write(0);
				break;	
		}
		pc.write(stvec.BASE() << 2);					// Synchronous exceptions are always DIRECT
	} else {	// Handle in M-Mode
		mcause.interrupt() = 0;
		mcause.exception_code() = code;
		mstatus.MPP() = (uint32_t)priv.get();	// Previous privilege
		priv.set(Privilege::Level::MACHINE);	// New privilege
		mstatus.MPIE() = mstatus.MIE();			// Previous interrupt-enable of target mode
		mstatus.MIE() = 0;						// Disable interrupt-enable of target mode
		mepc.write(pc.read()+4);				// Previous PC
		switch(code){
			case Exceptions::CODE::INSTRUCTION_ACCESS_FAULT:
			case Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED:
			case Exceptions::CODE::INSTRUCTION_PAGE_FAULT:
			case Exceptions::CODE::LOAD_ACCESS_FAULT:
			case Exceptions::CODE::LOAD_ADDRESS_MISALIGNED:
			case Exceptions::CODE::LOAD_PAGE_FAULT:
			case Exceptions::CODE::STORE_AMO_ACCESS_FAULT:
			case Exceptions::CODE::STORE_AMO_ADDRESS_MISALIGNED:
			case Exceptions::CODE::STORE_AMO_PAGE_FAULT:
			case Exceptions::CODE::BREAKPOINT:
				mtval.write(pc.read());
				break;
			case Exceptions::ILLEGAL_INSTRUCTION:
				mtval.write(instr.read());
				break;
			default:
				mtval.write(0);
				break;	
		}

		if(code != Exceptions::CODE::ECALL_FROM_UMODE)
			cout << "PE" << dec << router_addr << ": Exception code " << code << " with value " << mtval.read() << " at PC " << hex << pc.read() << " offset " << mrar.read() << endl;
		
		pc.write(mtvec.BASE() << 2);					// Synchronous exceptions are always DIRECT
	}
}

bool RiscV::decode()
{
	// wait(Timings::DECODE);
	// First level of decoding. Decode the opcode
	switch(instr.opcode()){
	case (uint32_t)Instructions::OPCODES::OP_IMM:
		return decode_op_imm();
		break;
	case (uint32_t)Instructions::OPCODES::LUI:
		execute = &RiscV::lui;
		move_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		move_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::OPCODES::AUIPC:
		execute = &RiscV::auipc;
		arith_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		arith_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::OPCODES::OP:
		return decode_op();
		break;
	case (uint32_t)Instructions::OPCODES::JAL:
		execute = &RiscV::jal;
		jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::OPCODES::JALR:
		switch(instr.funct3()){
		case (uint32_t)Instructions::FUNCT3::JALR:
			execute = &RiscV::jalr;
			jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	case (uint32_t)Instructions::OPCODES::BRANCH:
		return decode_branch();
		break;
	case (uint32_t)Instructions::OPCODES::LOAD:
		return decode_load();
		break;
	case (uint32_t)Instructions::OPCODES::STORE:
		return decode_store();
		break;
	case (uint32_t)Instructions::OPCODES::MISC_MEM:
		switch(instr.funct3()){
		case (uint32_t)Instructions::FUNCT3::FENCE:
			execute = &RiscV::fence;
			nop_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			nop_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	case (uint32_t)Instructions::OPCODES::SYSTEM:
		return decode_system();
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_op_imm()
{
	// Decodes the imm[11:5] and the funct3
	switch(instr.funct3()){
	case (uint32_t)Instructions::FUNCT3::ADDI:
		execute = &RiscV::addi;
		arith_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		arith_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::SLTI:
		execute = &RiscV::slti;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::SLTIU:
		execute = &RiscV::sltiu;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::XORI:
		execute = &RiscV::xori;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::ORI:
		execute = &RiscV::ori;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::ANDI:
		execute = &RiscV::andi;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::SLLI:
		switch(instr.funct7()){
		case (uint32_t)Instructions::FUNCT7::SLLI:
			execute = &RiscV::slli;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:		
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	case (uint32_t)Instructions::FUNCT3::SRLI_SRAI:
		switch(instr.funct7()){
		case (uint32_t)Instructions::FUNCT7::SRAI:
			execute = &RiscV::srai;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT7::SRLI:
			execute = &RiscV::srli;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_op()
{
	// Decodes funct7 first and then funct3
	switch(instr.funct7()){
	case (uint32_t)Instructions::FUNCT7::SUB_SRA:
		switch(instr.funct3()){
		case (uint32_t)Instructions::FUNCT3::SUB:
			execute = &RiscV::sub;
			arith_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			arith_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::SRA:
			execute = &RiscV::sra;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	case (uint32_t)Instructions::FUNCT7::MULDIV:
		switch(instr.funct3()){
		case (uint32_t)Instructions::FUNCT3::MUL:
			execute = &RiscV::mul;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::MULH:
			execute = &RiscV::mulh;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::MULHSU:
			execute = &RiscV::mulhsu;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::MULHU:
			execute = &RiscV::mulhu;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::DIV:
			execute = &RiscV::div;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::DIVU:
			execute = &RiscV::divu;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::REM:
			execute = &RiscV::rem;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::REMU:
			execute = &RiscV::remu;
			mult_div_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			mult_div_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	//case (uint32_t)Instructions::FUNCT7::ADD_SLT_SLTU:
	//case (uint32_t)Instructions::FUNCT7::AND_OR_XOR:
	//case (uint32_t)Instructions::FUNCT7::SLL_SRL:
	case 0:
		switch(instr.funct3()){
		case (uint32_t)Instructions::FUNCT3::ADD:
			execute = &RiscV::add;
			arith_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			arith_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::SLL:
			execute = &RiscV::sll;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::SLT:
			execute = &RiscV::slt;
			logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::SLTU:
			execute = &RiscV::sltu;
			logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::XOR:
			execute = &RiscV::_xor;
			logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::SRL:
			execute = &RiscV::srl;
			shift_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			shift_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::OR:
			execute = &RiscV::_or;
			logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		case (uint32_t)Instructions::FUNCT3::AND:
			execute = &RiscV::_and;
			logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
			logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_branch()
{
	switch(instr.funct3()){
	case (uint32_t)Instructions::FUNCT3::BEQ:
		execute = &RiscV::beq;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::BNE:
		execute = &RiscV::bne;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::BLT:
		execute = &RiscV::blt;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::BGE:
		execute = &RiscV::bge;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::BLTU:
		execute = &RiscV::bltu;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::BGEU:
		execute = &RiscV::bgeu;
		branch_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		branch_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_load()
{
	switch(instr.funct3()){
	case (uint32_t)Instructions::FUNCT3::LB:
		execute = &RiscV::lb;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::LH:
		execute = &RiscV::lh;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::LW:
		execute = &RiscV::lw;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::LBU:
		execute = &RiscV::lbu;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::LHU:
		execute = &RiscV::lhu;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_store()
{
	switch(instr.funct3()){
	case (uint32_t)Instructions::FUNCT3::SB:
		execute = &RiscV::sb;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::SH:
		execute = &RiscV::sh;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::SW:
		execute = &RiscV::sw;
		load_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		load_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::decode_system()
{
	switch(instr.funct3()){
	case (uint32_t)Instructions::FUNCT3::PRIV:
		switch(instr.funct7()){
		case (uint32_t)Instructions::FUNCT7::ECALL_EBREAK:
			if(!(instr.rs1() || instr.rd())){
				switch(instr.rs2()){
				case (uint32_t)Instructions::RS2::ECALL:
					execute = &RiscV::ecall;
					jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
					jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
					break;
				case (uint32_t)Instructions::RS2::EBREAK:
					execute = &RiscV::ebreak;
					jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
					jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
					break;
				default:
					handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
					return true;
				}
			} else {
				handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
				return true;
			}
			break;
		case (uint32_t)Instructions::FUNCT7::SRET_WFI:
			if(!(instr.rs1() || instr.rd())){
				switch(instr.rs2()){
				case (uint32_t)Instructions::RS2::RET:
					execute = &RiscV::sret;
					jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
					jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
					break;
				case (uint32_t)Instructions::RS2::WFI:
					execute = &RiscV::wfi;
					other_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
					other_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
					break;
				default:
					handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
					return true;
				}
			} else {
				handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
				return true;
			}
			break;
		case (uint32_t)Instructions::FUNCT7::MRET:
			if(!(instr.rs1() || instr.rd()) && instr.rs2() == (uint32_t)Instructions::RS2::RET){
				execute = &RiscV::mret;
				jump_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
				jump_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			} else {
				handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
				return true;
			}
			break;
		case (uint32_t)Instructions::FUNCT7::SFENCE_VMA:
			if(!instr.rd()){
				execute = &RiscV::sfence_vma;
				other_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
				other_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
			} else {
				handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
				return true;
			}
			break;
		default:
			handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
			return true;
		}
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRW:
		execute = &RiscV::csrrw;
		move_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		move_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRS:
		execute = &RiscV::csrrs;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRC:
		execute = &RiscV::csrrc;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRWI:
		execute = &RiscV::csrrwi;
		move_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		move_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRSI:
		execute = &RiscV::csrrsi;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	case (uint32_t)Instructions::FUNCT3::CSRRCI:
		execute = &RiscV::csrrci;
		logical_inst_kernel += (int)(priv.get() != Privilege::Level::USER);
		logical_inst_tasks += (int)(priv.get() == Privilege::Level::USER);
		break;
	default:
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	return false;
}

bool RiscV::lui()
{
	// wait(Timings::LOGICAL);
	x[instr.rd()].range(31,12) = instr.imm_31_12();
	x[instr.rd()].range(11,0) = 0;
	return false;
}

bool RiscV::auipc()
{
	// wait(Timings::LOGICAL);
	Register r;
	r.range(31,12) = instr.imm_31_12();
	r.range(11,0) = 0;
	r.write(r.read() + pc.read());
	x[instr.rd()] = r;
	return false;
}

bool RiscV::jal()
{
	// wait(Timings::LOGICAL);

	// Sign-extend offset
	Register r;
	r.range(31,20) = (int)instr.bit(31) * -1;
	
	r.range(19, 12) = instr.imm_19_12();
	r.bit(11) = instr.imm_11_J();
	r.range(10, 1) = instr.imm_10_1();
	r.bit(0) = 0;
	
	r.write(r.read() + pc.read());

	// Save PC ("Link")
	x[instr.rd()].write(pc.read()+4);

	if(r.read() % 4)
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
	else
		pc.write(r.read());
	

	return true;
}

bool RiscV::jalr()
{
	// wait(Timings::LOGICAL);

	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());
	r.bit(0) = 0;

	// Save PC ("Link")
	x[instr.rd()].write(pc.read()+4);

	if(r.read() % 4)
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
	else
		pc.write(r.read());

	return true;
}

bool RiscV::beq()
{
	// wait(Timings::LOGICAL);
	if(x[instr.rs1()].read() == x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::bne()
{
	// wait(Timings::LOGICAL);
	if(x[instr.rs1()].read() != x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::blt()
{
	// wait(Timings::LOGICAL);
	if((int)x[instr.rs1()].read() < (int)x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::bge()
{
	// wait(Timings::LOGICAL);
	if((int)x[instr.rs1()].read() >= (int)x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::bltu()
{
	// wait(Timings::LOGICAL);
	if((unsigned int)x[instr.rs1()].read() < (unsigned int)x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::bgeu()
{
	// wait(Timings::LOGICAL);
	if((unsigned int)x[instr.rs1()].read() >= (unsigned int)x[instr.rs2()].read()){ // Taken
		// Sign-extend offset
		Register r;
		r.range(31,12) = (int)instr.bit(31) * -1;
		r.bit(11) = instr.imm_11_B();
		r.range(10, 5) = instr.imm_10_5();
		r.range(4, 1) = instr.imm_4_1();
		r.bit(0) = 0;
		r.write(r.read() + pc.read());

		if(r.read() % 4)
			handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		else
			pc.write(r.read());
		return true;
	} else { // Not taken
		return false;
	}
}

bool RiscV::lb()
{
	// wait(Timings::LOGICAL);
	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Load Byte must be 8-bit aligned
	const uint32_t offset = r.read() & 0x00000003;

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::LOAD_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	if(offset == 3){	// MSB
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(31, 24);
	} else if(offset == 2){
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(23, 16);
	} else if(offset){
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(15, 8);
	} else {			// LSB
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(7, 0);
	}
	x[instr.rd()].range(31, 8) = -1 * x[instr.rd()].bit(7);	// Sign-extended

	return false;
}

bool RiscV::lh()
{
	// wait(Timings::LOGICAL);
	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Load Half must be 16-bit aligned
	const uint32_t offset = r.read() & 0x00000003;
	if(offset % 2){
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		return true;
	}

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::LOAD_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	if(!offset){	// LSW
		x[instr.rd()].range(15, 0) = mem_read(phy_addr.read()).range(15, 0);
	} else {		// MSW
		x[instr.rd()].range(15, 0) = mem_read(phy_addr.read()).range(31, 16);
	}
	x[instr.rd()].range(31, 16) = -1 * x[instr.rd()].bit(15);	// Sign-extended

	return false;
}

bool RiscV::lw()
{
	// wait(Timings::LOGICAL);
	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Load Word must be 32-bit aligned
	const uint32_t offset = r.read() & 0x00000003;
	if(offset){
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		return true;
	}

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::LOAD_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	x[instr.rd()].write(mem_read(phy_addr.read()));
	return false;
}

bool RiscV::lbu()
{
	// wait(Timings::LOGICAL);
	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Load Byte must be 8-bit aligned
	const uint32_t offset = r.read() & 0x00000003;

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::LOAD_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	if(offset == 3){	// MSB
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(31, 24);
	} else if(offset == 2){
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(23, 16);
	} else if(offset){
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(15, 8);
	} else {			// LSB
		x[instr.rd()].range(7, 0) = mem_read(phy_addr.read()).range(7, 0);
	}
	x[instr.rd()].range(31, 8) = 0;	// 0-extended

	return false;
}

bool RiscV::lhu()
{
	// wait(Timings::LOGICAL);
	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Load Half must be 16-bit aligned
	const uint32_t offset = r.read() & 0x00000003;
	if(offset % 2){
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		return true;
	}

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::LOAD_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	if(!offset){	// LSW
		x[instr.rd()].range(15, 0) = mem_read(phy_addr.read()).range(15, 0);
	} else {		// MSW
		x[instr.rd()].range(15, 0) = mem_read(phy_addr.read()).range(31, 16);
	}
	x[instr.rd()].range(31, 16) = 0;	// 0-extended

	return false;
}

bool RiscV::sb()
{
	// wait(Timings::LOGICAL);

	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 5) = instr.imm_11_5();
	r.range(4, 0) = instr.imm_4_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Store Byte must be 8-bit aligned
	uint32_t offset = r.read() & 0x00000003;

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::STORE_AMO_PAGE_FAULT))
		return true;

	// This seem odd, but this image can help understand:
	// https://upload.wikimedia.org/wikipedia/commons/e/ed/Little-Endian.svg
	if(offset == 3)
		offset = 0x8;	// MSB
	else if(offset == 2)
		offset = 0x4;
	else if(offset)
		offset = 0x2;
	else
		offset = 0x1;	// LSB

	uint32_t byte_write = x[instr.rs2()].read().range(7, 0);
	byte_write |= (byte_write << 24) | (byte_write << 16) | (byte_write << 8);

	while(mem_pause.read())
		wait(1);

	mem_write(phy_addr.read(), byte_write, offset);

	return false;
}

bool RiscV::sh()
{
	// wait(Timings::LOGICAL);

	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 5) = instr.imm_11_5();
	r.range(4, 0) = instr.imm_4_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Store Half must be 16-bit aligned
	uint32_t offset = r.read() & 0x00000003;
	if(offset % 2){
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		return true;
	}

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::STORE_AMO_PAGE_FAULT))
		return true;

	if(!offset)
		offset = 0x3;	// LSW
	else
		offset = 0xC;	// MSW

	uint32_t byte_write = x[instr.rs2()].read().range(15, 0);
	byte_write |= (byte_write << 16);

	while(mem_pause.read())
		wait(1);

	mem_write(phy_addr.read(), byte_write, offset);

	return false;
}

bool RiscV::sw()
{
	// wait(Timings::LOGICAL);

	// Sign-extend offset
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 5) = instr.imm_11_5();
	r.range(4, 0) = instr.imm_4_0();
	r.write(r.read() + x[instr.rs1()].read());

	// Store Word must be 32-bit aligned
	const uint32_t offset = r.read() & 0x00000003;
	if(offset){
		handle_exceptions(Exceptions::CODE::INSTRUCTION_ADDRESS_MISALIGNED);
		return true;
	}

	Address vir_addr;
	vir_addr.write((r.read() & 0xFFFFFFFC));

	Address phy_addr;
	if(paging(vir_addr, phy_addr, Exceptions::CODE::STORE_AMO_PAGE_FAULT))
		return true;

	while(mem_pause.read())
		wait(1);

	mem_write(phy_addr.read(), x[instr.rs2()].read(), 0xF);

	return false;
}

bool RiscV::addi()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(x[instr.rs1()].read() + r.read());

	return false;
}

bool RiscV::slti()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(((int)x[instr.rs1()].read() < (int)r.read()));

	return false;
}

bool RiscV::sltiu()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(((unsigned int)x[instr.rs1()].read() < (unsigned int)r.read()));

	return false;
}

bool RiscV::xori()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(x[instr.rs1()].read() ^ r.read());

	return false;
}

bool RiscV::ori()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(x[instr.rs1()].read() | r.read());

	return false;
}

bool RiscV::andi()
{
	// wait(Timings::LOGICAL);
	// Sign-extend immediate
	Register r;
	r.range(31,12) = (int)instr.bit(31) * -1;
	r.range(11, 0) = instr.imm_11_0();

	x[instr.rd()].write(x[instr.rs1()].read() & r.read());

	return false;
}

bool RiscV::slli()
{
	// wait(Timings::LOGICAL);
	// rs2 is imm[4:0] for shift
	x[instr.rd()].write(x[instr.rs1()].read() << instr.rs2());

	return false;
}

bool RiscV::srli()
{
	// wait(Timings::LOGICAL);
	// rs2 is imm[4:0] for shift
	x[instr.rd()].write(x[instr.rs1()].read() >> instr.rs2());

	return false;
}

bool RiscV::srai()
{
	// wait(Timings::LOGICAL);

	Register r;
	r.write(x[instr.rs1()].read());

	// Sign-extend
	uint32_t sign = 0xFFFFFFFF * r.bit(31);
	// rs2 is imm[4:0] for shift
	r.write(r.read() >> instr.rs2());
	r.range(31, 31 - instr.rs2()) = sign;

	x[instr.rd()].write(r.read());

	return false;
}

bool RiscV::add()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() + x[instr.rs2()].read());

	return false;
}

bool RiscV::sub()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() - x[instr.rs2()].read());

	return false;
}

bool RiscV::sll()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() << x[instr.rs2()].range(4, 0));

	return false;
}

bool RiscV::slt()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(((int)x[instr.rs1()].read() < (int)x[instr.rs2()].read()));

	return false;
}

bool RiscV::sltu()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(((unsigned int)x[instr.rs1()].read() < (unsigned int)x[instr.rs2()].read()));

	return false;
}

bool RiscV::_xor()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() ^ x[instr.rs2()].read());

	return false;
}

bool RiscV::srl()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() >> x[instr.rs2()].range(4,0));

	return false;
}

bool RiscV::sra()
{
	// wait(Timings::LOGICAL);

	Register r;
	r.write(x[instr.rs1()].read());

	// Sign-extend
	uint32_t sign = 0xFFFFFFFF * r.bit(31);
	r.write(r.read() >> x[instr.rs2()].range(4, 0));
	r.range(31, 31 - x[instr.rs2()].range(4, 0)) = sign;

	x[instr.rd()].write(r.read());

	return false;
}

bool RiscV::_or()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() | x[instr.rs2()].read());

	return false;
}

bool RiscV::_and()
{
	// wait(Timings::LOGICAL);

	x[instr.rd()].write(x[instr.rs1()].read() & x[instr.rs2()].read());

	return false;
}

bool RiscV::fence()
{
	// wait(Timings::LOGICAL);

	return false;
}

bool RiscV::ecall()
{
	// wait(Timings::LOGICAL);

	switch(priv.get()){
	case Privilege::Level::MACHINE:
		handle_exceptions(Exceptions::CODE::ECALL_FROM_MMODE);
		break;
	case Privilege::Level::SUPERVISOR:
		handle_exceptions(Exceptions::CODE::ECALL_FROM_SMODE);
		break;
	case Privilege::Level::USER:
		handle_exceptions(Exceptions::CODE::ECALL_FROM_UMODE);
		break;
	}	

	return true;
}

bool RiscV::ebreak()
{
	// wait(Timings::LOGICAL);

	return false;
}

bool RiscV::mul()
{
	// wait(Timings::MUL);

	x[instr.rd()].write((uint32_t)x[instr.rs1()].read() * (uint32_t)x[instr.rs2()].read());

	return false;
}

bool RiscV::mulh()
{
	// wait(Timings::MUL);

	uint64_t res = (int64_t)x[instr.rs1()].read() * (int64_t)x[instr.rs2()].read();
	uint32_t high = res >> 32;
	
	x[instr.rd()].write(high);

	return false;
}

bool RiscV::mulhsu()
{
	// wait(Timings::MUL);

	uint64_t res = (int64_t)x[instr.rs1()].read() * (uint64_t)x[instr.rs2()].read();
	uint32_t high = res >> 32;
	
	x[instr.rd()].write(high);

	return false;
}

bool RiscV::mulhu()
{
	// wait(Timings::MUL);

	uint64_t res = (uint64_t)x[instr.rs1()].read() * (uint64_t)x[instr.rs2()].read();
	uint32_t high = res >> 32;
	
	x[instr.rd()].write(high);

	return false;
}

bool RiscV::div()
{
	// wait(Timings::MUL);

	if(!x[instr.rs1()].read()){ // 0 divided by anything is 0
		x[instr.rd()].write(0);
	} else if(!x[instr.rs2()].read()){ // Div by 0
		x[instr.rd()].write((uint32_t)-1); // Max out
	} else if(x[instr.rs1()].read() == 1 && (int32_t)x[instr.rs2()].read() == -1){ // Overflow
		x[instr.rd()].write(x[instr.rs1()].read()); // Writes the dividend
	} else {
		x[instr.rd()].write((int32_t)x[instr.rs1()].read() / (int32_t)x[instr.rs2()].read());
	}

	return false;
}

bool RiscV::divu()
{
	// wait(Timings::MUL);

	if(!x[instr.rs1()].read()){ // 0 divided by anything is 0
		x[instr.rd()].write(0);
	} else if(!x[instr.rs2()].read()){ // Div by 0
		x[instr.rd()].write((uint32_t)-1); // Max out
	} else {
		x[instr.rd()].write((uint32_t)x[instr.rs1()].read() / (uint32_t)x[instr.rs2()].read());
	}

	return false;
}

bool RiscV::rem()
{
	// wait(Timings::MUL);

	if(!x[instr.rs1()].read()){ // 0 divided by anything is 0
		x[instr.rd()].write(0);
	} else if(!x[instr.rs2()].read()){ // Div by 0
		x[instr.rd()].write(x[instr.rs1()].read()); // Writes the dividend
	} else if(x[instr.rs1()].read() == 1 && (int32_t)x[instr.rs2()].read() == -1){ // Overflow
		x[instr.rd()].write(0); // No remainder
	} else {
		x[instr.rd()].write((int32_t)x[instr.rs1()].read() % (int32_t)x[instr.rs2()].read());
	}

	return false;
}

bool RiscV::remu()
{
	// wait(Timings::MUL);

	if(!x[instr.rs1()].read()){ // 0 divided by anything is 0
		x[instr.rd()].write(0);
	} else if(!x[instr.rs2()].read()){ // Div by 0
		x[instr.rd()].write(x[instr.rs1()].read()); // Writes the dividend
	} else {
		x[instr.rd()].write((uint32_t)x[instr.rs1()].read() % (uint32_t)x[instr.rs2()].read());
	}

	return false;
}

bool RiscV::csrrw()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool wo = false;
	if(!instr.rd())	// WO instruction
		wo = true;

	if(csr_helper(instr.imm_11_0(), true, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	// Initial value of rs1 in case rd==rs1.
	Register buffer;
	buffer.write(x[instr.rs1()].read());

	if(!wo)
		x[instr.rd()].write(csr->read() & rm);

	csr->write((buffer.read() & wmand) | wmor);

	return false;
}

bool RiscV::csrrs()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool rw = true;
	if(!instr.rs1())	// RO instruction
		rw = false;

	if(csr_helper(instr.imm_11_0(), rw, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	// Initial value of rs1 in case rd==rs1.
	Register buffer;
	buffer.write(x[instr.rs1()].read());

	x[instr.rd()].write(csr->read() & rm);
	if(rw)
		csr->write(((x[instr.rd()].read() | buffer.read()) & wmand) | wmor);

	return false;
}

bool RiscV::csrrc()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool rw = true;
	if(!instr.rs1())	// RO instruction
		rw = false;

	if(csr_helper(instr.imm_11_0(), rw, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	// Initial value of rs1 in case rd==rs1.
	Register buffer;
	buffer.write(x[instr.rs1()].read());

	x[instr.rd()].write(csr->read() & rm);
	if(rw)
		csr->write(((x[instr.rd()].read() & ~buffer.read()) & wmand) | wmor);

	return false;
}

bool RiscV::csrrwi()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool wo = false;
	if(!instr.rd())	// WO instruction
		wo = true;

	if(csr_helper(instr.imm_11_0(), true, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	if(!wo)
		x[instr.rd()].write(csr->read() & rm);
	csr->write(((uint32_t)instr.rs1() & wmand) | wmor);

	return false;
}

bool RiscV::csrrsi()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool rw = true;
	if(!instr.rs1())	// RO instruction
		rw = false;

	if(csr_helper(instr.imm_11_0(), rw, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	x[instr.rd()].write(csr->read() & rm);
	if(rw)
		csr->write(((x[instr.rd()].read() | (uint32_t)instr.rs1()) & wmand) | wmor);

	return false;
}

bool RiscV::csrrci()
{
	// wait(Timings::CSR);

	Register *csr = nullptr;
	uint32_t wmand = -1;
	uint32_t wmor  =  0;
	uint32_t rm    = -1;

	bool rw = true;
	if(!instr.rs1())	// RO instruction
		rw = false;

	if(csr_helper(instr.imm_11_0(), rw, csr, wmand, wmor, rm))
		return true;	// Exception has been raised.

	x[instr.rd()].write(csr->read() & rm);
	if(rw)
		csr->write(((x[instr.rd()].read() & ~(uint32_t)instr.rs1()) & wmand) | wmor);

	return false;
}

bool RiscV::sret()
{
	// wait(Timings::LOGICAL);

	// Can only be called in M and S-Mode and if SRET trap is disabled
	if(priv.get() == Privilege::Level::USER || mstatus.TSR()){
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
	} else {
		// Manipulate the privilege stack
		mstatus.SIE() = mstatus.SPIE();
		priv.set((Privilege::Level)((uint32_t)mstatus.SPP()));
		mstatus.SPIE() = 1;
		mstatus.SPP() = (uint32_t)Privilege::Level::USER;

		// Return to exception pc
		pc.write(sepc.read());
	}

	return true;
}

bool RiscV::mret()
{
	// wait(Timings::LOGICAL);

	// Can only be called in M-Mode
	if(priv.get() != Privilege::Level::MACHINE){
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
	} else {
		// Manipulate the privilege stack
		mstatus.MIE() = mstatus.MPIE();
		priv.set((Privilege::Level)((uint32_t)mstatus.MPP()));
		mstatus.MPIE() = 1;
		mstatus.MPP() = (uint32_t)Privilege::Level::USER;

		// Return to exception pc
		pc.write(mepc.read());
	}

	return true;
}

bool RiscV::wfi()
{
	// wait(Timings::LOGICAL);

	// Not available in U-Mode or if timeout wait in S-Mode
	if(priv.get() == Privilege::Level::USER || (mstatus.TW() && priv.get() == Privilege::Level::SUPERVISOR)){
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}
	
	// NOP

	return false;
}

bool RiscV::sfence_vma()
{
	// wait(Timings::LOGICAL);

	if(priv.get() == Privilege::Level::USER){
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}

	// NOP

	return false;
}

bool RiscV::csr_helper(uint16_t addr, bool rw, Register* &csr, uint32_t &wmask_and, uint32_t &wmask_or, uint32_t &rmask)
{
	// Access CSR
	csr 		= nullptr;
	wmask_and 	= 0xFFFFFFFF;
	wmask_or	= 0x00000000;
	rmask 		= 0xFFFFFFFF;

	// Check R/W permissions
	const uint16_t rwmask = addr & CSR::RWA_MASK;
	if(rwmask == CSR::RO && rw){	// Read-only
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}

	// Check privilege permission
	const uint8_t level = (addr & CSR::LVL_MASK) >> CSR::LVL_SHIFT;
	if(level < (uint8_t)priv.get()){	// Not enough privilege
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}

	switch(addr){
	case CSR::Address::MISA:
		csr = &misa;
		wmask_and = 0x3C000000;	// Only write to empty field.
		wmask_or  = 0x40141100; // 32-bit MSU
		break;
	case CSR::Address::MVENDORID:
		csr = &mvendorid;	// already r-o
		break;
	case CSR::Address::MARCHID:
		csr = &marchid;	// already r-o
		break;
	case CSR::Address::MIMPID:
		csr = &mimpid;	// already r-o
		break;
	case CSR::Address::MHARTID:
		csr = &mhartid;	// already r-o
		break;
	case CSR::Address::MSTATUS:
		// @todo Block value 0b10 to be written to MPP
		csr = &mstatus;
		wmask_and = 0x007E19AA;	// WPRIV + FS off + UIE/UPIE 0 + XS ro + SD off
		rmask = 0x007E19AA;	// WPRIV + FS off + UIE/UPIE 0 + XS off + SD off
		break;
	case CSR::Address::MTVEC:
		// @todo Block value 0x11 to be written to MODE
		csr = &mtvec;
		wmask_and = 0xFFFFFFFC; // BASE 4B aligned
		break;
	case CSR::Address::MEDELEG:
		csr = &medeleg;
		wmask_and = 0xFFFFF7FF; // medeleg[11] 0
		rmask = 0xFFFFF7FF; // medeleg[11] 0
		break;
	case CSR::Address::MIDELEG:	
		csr = &mideleg;
		wmask_and = 0xFFFFFEEE; // Disable U interrupts
		rmask = 0xFFFFFEEE; // Disable U interrupts
		break;
	case CSR::Address::MIP:
		csr = &mip;
		wmask_and = 0x00000222;	// WPRIV + U off + MTIP/MSIP/MEIP ro
		rmask = 0x00000AAA;	// WPRIV + U off
		break;
	case CSR::Address::MIE:
		csr = &mie;
		wmask_and = 0x00000AAA;	// WPRIV + U off
		rmask = 0x00000AAA;	// WPRIV + U off
		break;
	case CSR::Address::MSCRATCH:
		csr = &mscratch;
		break;
	case CSR::Address::MEPC:
		csr = &mepc;
		wmask_and = 0xFFFFFFFC; // 4B align
		break;
	case CSR::Address::MCAUSE:
		csr = &mcause;	// WLRL
		break;
	case CSR::Address::MTVAL:
		csr = &mtval;
		break;
	case CSR::Address::SSTATUS: // SSTATUS is restricted view of MSTATUS
		csr = &mstatus;
		wmask_and = 0x000C0122;	// WPRIV + FS off + UIE/UPIE 0 + XS ro + SD off
		rmask = 0x000C0122;	// WPRIV + FS off + UIE/UPIE 0 + XS off + SD off
		break;
	case CSR::Address::STVEC:
		// @todo Block value 0x11 to be written to MODE
		csr = &stvec;
		wmask_and = 0xFFFFFFFC; // BASE 4B aligned
		break;
	case CSR::Address::SIP:	// SIP is restricted view of MIP
		csr = &mip;
		wmask_and = 0x00000222;	// WPRIV + U off
		rmask = 0x00000222;	// WPRIV + U off
		break;
	case CSR::Address::SIE:	// SIE is restricted view of MIE
		csr = &mie;
		wmask_and = 0x00000222;	// WPRIV + U off
		rmask = 0x00000222;	// WPRIV + U off
		break;
	case CSR::Address::SSCRATCH:
		csr = &sscratch;
		break;
	case CSR::Address::SEPC:
		csr = &sepc;
		wmask_and = 0xFFFFFFFC; // 4B align
		break;
	case CSR::Address::SCAUSE:
		csr = &scause;	// WLRL
		break;
	case CSR::Address::STVAL:
		csr = &stval;
		break;
	case CSR::Address::SATP:
		csr = &satp;
		break;
	case CSR::Address::MRAR:
		csr = &mrar;
		break;
	default:	// Unknown CSR
		handle_exceptions(Exceptions::CODE::ILLEGAL_INSTRUCTION);
		return true;
	}

	return false;
}