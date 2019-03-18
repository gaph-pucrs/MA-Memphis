//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  test_bench.cpp
//
//  Brief description: Testbench debugger
//
//------------------------------------------------------------------------------------------------

#include "test_bench.h"

#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(test_bench);
#endif

void test_bench::ClockGenerator(){
	while(1){
		clock.write(0);
		wait (5, SC_NS);					//Allow signals to set
		clock.write(1);
		wait (5, SC_NS);					//Allow signals to set
	}
}
	
void test_bench::resetGenerator(){
	reset.write(1);
	wait (70, SC_NS);
	reset.write(0);
}

	
