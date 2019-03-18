//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  mlite_cpu.h
//
//  Brief description: This source has the module mlite_cpu used in mlite_cpu.cpp
//
//------------------------------------------------------------------------------------------------

#ifndef _mlite_cpu_h
#define _mlite_cpu_h

#include <systemc.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../../../standards.h"


  #define TAM_FLIT 32
  typedef sc_uint<(TAM_FLIT/2)> regmetadeflit;

typedef struct {
   long int r[32];
   unsigned long int pc, epc, global_inst_reg;
   long int hi;
   long int lo;
} State;


SC_MODULE(mlite_cpu) {

	sc_in< bool > clk;
	sc_in< bool > reset_in;
	sc_in< bool > intr_in;
	sc_in< bool > mem_pause;

	sc_out< sc_uint<32> > mem_address;
	sc_out< sc_uint<32> > mem_data_w;
	sc_in < sc_uint<32> > mem_data_r;
	sc_out< sc_uint<4> >  mem_byte_we;

	sc_out< sc_uint<8> >  current_page;


	State *state, state_instance;

	unsigned int opcode, prefetched_opcode, pc_last;
	unsigned int op, rs, rt, rd, re, func, imm, target;
	int imm_shift;
	int *r, word_addr;
	unsigned int *u;
	unsigned int ptr, page, byte_write;
	unsigned char big_endian, shift;
	sc_uint<4> byte_en;

	bool intr_enable, prefetch, jump_or_branch, no_execute_branch_delay_slot;

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
 
	/*** Process function ***/
	void mlite();

	/*** Helper functions ***/
	void mult_big(unsigned int a, unsigned int b);
	void mult_big_signed(int a, int b);
	
	SC_HAS_PROCESS(mlite_cpu);
	mlite_cpu(sc_module_name name_, regmetadeflit address_router_ = 0) :
	sc_module(name_), address_router(address_router_)
	{

		SC_THREAD(mlite);
		sensitive << clk.pos() << mem_pause.pos();
		sensitive << mem_pause.neg();

		state = &state_instance;

		// MIPS: Big endian.
		big_endian = 1;

		// Used to generate the 'current_page' signal from the 'page' signal
		shift = (unsigned char) (log10(PAGE_SIZE_BYTES)/log10(2));
	}	
	private:
		regmetadeflit address_router;
};

#endif
