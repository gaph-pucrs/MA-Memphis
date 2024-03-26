//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  test_bench.h
//
//  Brief description: Testbench debugger
//
//------------------------------------------------------------------------------------------------

#include <systemc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

using namespace std;

#include "Memphis.hpp"
#include "AppInjector.hpp"
#include "MAInjector.hpp"

SC_MODULE(test_bench) {
	
	sc_signal< bool >	clock;
	sc_signal< bool >	reset;
		
	//IO signals connecting App Injector and Memphis
	sc_signal<bool>		memphis_injector_tx;
	sc_signal<bool>		memphis_injector_credit_i;
	sc_signal<sc_uint<TAM_FLIT+1> > 	memphis_injector_data_out;
	sc_signal<bool>		memphis_injector_rx;
	sc_signal<bool>		memphis_injector_credit_o;
	sc_signal<sc_uint<TAM_FLIT+1> >	memphis_injector_data_in;

	//Create the signals of your IO component here:
	sc_signal<bool>		memphis_mai_tx;
	sc_signal<bool>		memphis_mai_credit_i;
	sc_signal<sc_uint<TAM_FLIT+1> > 	memphis_mai_data_out;
	sc_signal<bool>		memphis_mai_rx;
	sc_signal<bool>		memphis_mai_credit_o;
	sc_signal<sc_uint<TAM_FLIT+1> >	memphis_mai_data_in;


	//Clock and Reset processes
	void ClockGenerator();
	void resetGenerator();
	
	Memphis * MPSoC;
	AppInjector * io_app;
	MAInjector * ma_app;

	char aux[255];
	FILE *fp;
	
	std::string path;
	
	SC_HAS_PROCESS(test_bench);
	test_bench(sc_module_name name_, std::string program_path = "") :
    sc_module(name_)
    {
		path = program_path.substr(0, program_path.find_last_of("/"));
		fp = 0;

		MPSoC = new Memphis("Memphis", path);
		MPSoC->clock(clock);
		MPSoC->reset(reset);
		MPSoC->memphis_app_injector_tx(memphis_injector_tx);
		MPSoC->memphis_app_injector_credit_i(memphis_injector_credit_i);
		MPSoC->memphis_app_injector_data_out(memphis_injector_data_out);
		MPSoC->memphis_app_injector_rx(memphis_injector_rx);
		MPSoC->memphis_app_injector_credit_o(memphis_injector_credit_o);
		MPSoC->memphis_app_injector_data_in(memphis_injector_data_in);
   
    	MPSoC->memphis_ma_injector_tx(memphis_mai_tx);
		MPSoC->memphis_ma_injector_credit_i(memphis_mai_credit_i);
		MPSoC->memphis_ma_injector_data_out(memphis_mai_data_out);
		MPSoC->memphis_ma_injector_rx(memphis_mai_rx);
		MPSoC->memphis_ma_injector_credit_o(memphis_mai_credit_o);
		MPSoC->memphis_ma_injector_data_in(memphis_mai_data_in);


		io_app = new AppInjector("App_Injector", path);
		io_app->clock(clock);
		io_app->reset(reset);
		io_app->rx(memphis_injector_tx);
		io_app->data_in(memphis_injector_data_out);
		io_app->credit_out(memphis_injector_credit_i);
		io_app->tx(memphis_injector_rx);
		io_app->data_out(memphis_injector_data_in);
		io_app->credit_in(memphis_injector_credit_o);

		ma_app = new MAInjector("MAInjector", path);
		ma_app->clock(clock);
		ma_app->reset(reset);
		ma_app->rx(memphis_mai_tx);
		ma_app->data_in(memphis_mai_data_out);
		ma_app->credit_out(memphis_mai_credit_i);
		ma_app->tx(memphis_mai_rx);
		ma_app->data_out(memphis_mai_data_in);
		ma_app->credit_in(memphis_mai_credit_o);

		SC_THREAD(ClockGenerator);
		SC_THREAD(resetGenerator);
	}
	~test_bench(){

		//Store in aux the c's string way, in address how many char were builded
		sprintf(aux, "%s/log_energy.txt", path.c_str());
		// Open a file called aux deferred on append mode
		fp = fopen (aux, "w+");
		fclose (fp);	

		for(int j=0;j<N_PE;j++){
			//Store in aux the c's string way, in address how many char were builded
			sprintf(aux, "%s/log_energy.txt", path.c_str());
			// Open a file called aux deferred on append mode
			fp = fopen (aux, "a");
			
			sprintf(aux, "router %d port EAST flits %d ", j, MPSoC-> pe[j]->router->wire_EAST);
			fprintf(fp,"%s",aux);
			sprintf(aux, "port WEST flits %d ",MPSoC-> pe[j] ->router->wire_WEST);
			fprintf(fp,"%s",aux);
			sprintf(aux, "port NORTH flits %d ",MPSoC-> pe[j] ->router->wire_NORTH);
			fprintf(fp,"%s",aux);
			sprintf(aux, "port SOUTH flits %d ",MPSoC-> pe[j] ->router->wire_SOUTH);
			fprintf(fp,"%s",aux);
			sprintf(aux, "port LOCAL flits %d ",MPSoC-> pe[j] ->router->wire_LOCAL);
			fprintf(fp,"%s",aux);
			sprintf(aux, "TOTAL flits %d ",MPSoC-> pe[j] ->router->total_flits);
			fprintf(fp,"%s",aux);
			sprintf(aux, "NUMBER pck %d ",MPSoC-> pe[j] ->router->mySwitchControl->number_pck);
			fprintf(fp,"%s",aux);


			sprintf(aux, "fluxo 0 = %d ", MPSoC-> pe[j]  ->router->fluxo_0);
			fprintf(fp,"%s",aux);
			sprintf(aux, "fluxo 1 = %d ",MPSoC-> pe[j]->router->fluxo_1);
			fprintf(fp,"%s",aux);
			sprintf(aux, "fluxo 2 =  %d ",MPSoC-> pe[j]->router->fluxo_2);
			fprintf(fp,"%s",aux);
			sprintf(aux, "fluxo 3 = %d ",MPSoC-> pe[j]->router->fluxo_3);
			fprintf(fp,"%s",aux);
			sprintf(aux, "fluxo 4 = %d ",MPSoC-> pe[j]->router->fluxo_4);
			fprintf(fp,"%s",aux);
			sprintf(aux, "fluxo 5 = %d\n",MPSoC-> pe[j]->router->fluxo_5);
			fprintf(fp,"%s",aux);
			fclose (fp);
					
		}
		
		for(int j=0;j<N_PE;j++)
		{
			
			//Store in aux the c's string way, in address how many char were builded
			sprintf(aux, "%s/log_energy.txt", path.c_str());
			// Open a file called aux deferred on append mode
			fp = fopen (aux, "a");

			sprintf(aux, "PROC %d Arithmetic_tasks %lu ",j, MPSoC-> pe[j] ->cpu.arith_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Load-Store_tasks %lu ", MPSoC-> pe[j] ->cpu.load_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Logical_tasks %lu ",MPSoC-> pe[j] ->cpu.logical_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Move_tasks %lu ",MPSoC-> pe[j] ->cpu.move_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Shift_tasks %lu ",MPSoC-> pe[j] ->cpu.shift_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Jump_tasks %lu ",MPSoC-> pe[j] ->cpu.jump_inst_tasks);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Branch_tasks %lu ",MPSoC-> pe[j] ->cpu.branch_inst_tasks);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Nop_tasks %lu ",MPSoC-> pe[j] ->cpu.nop_inst_tasks);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Mult-Div_tasks %lu ",MPSoC-> pe[j] ->cpu.mult_div_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Other_tasks %lu ",MPSoC-> pe[j] ->cpu.other_inst_tasks);
			fprintf(fp,"%s",aux);

			sprintf(aux, "Arithmetic_kernel %lu ",MPSoC-> pe[j] ->cpu.arith_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Load-Store_kernel %lu ", MPSoC-> pe[j] ->cpu.load_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Logical_kernel %lu ",MPSoC-> pe[j] ->cpu.logical_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Move_kernel %lu ",MPSoC-> pe[j] ->cpu.move_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Shift_kernel %lu ",MPSoC-> pe[j] ->cpu.shift_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Jump_kernel %lu ",MPSoC-> pe[j] ->cpu.jump_inst_kernel);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Branch_kernel %lu ",MPSoC-> pe[j] ->cpu.branch_inst_kernel);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Nop_kernel %lu ",MPSoC-> pe[j] ->cpu.nop_inst_kernel);
			fprintf(fp,"%s",aux);				
			sprintf(aux, "Mult-Div_kernel %lu ",MPSoC-> pe[j] ->cpu.mult_div_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Other_kernel %lu ",MPSoC-> pe[j] ->cpu.other_inst_kernel);
			fprintf(fp,"%s",aux);

			sprintf(aux, "Kernel %lu ",MPSoC-> pe[j] ->cpu.global_inst_kernel);
			fprintf(fp,"%s",aux);
			sprintf(aux, "Task %lu ",MPSoC-> pe[j] ->cpu.global_inst_tasks);
			fprintf(fp,"%s",aux);
			sprintf(aux, "TOTAL %lu\n",MPSoC-> pe[j] ->cpu.global_inst);
			fprintf(fp,"%s",aux);
			fclose (fp);

		}
					
	}
};


#ifndef MTI_SYSTEMC

int sc_main(int argc, char *argv[])
{
	int timeout = -1;

	if(argc == 3){
		for(int i = 0; i < argc; i++){
			if(argv[i][0] == '-'){
				switch(argv[i][1]){
					case 't':
						timeout = std::stoi(argv[++i]);
						break;
					default:
						cout << "Syntax: " << argv[0] << " [-t <milliseconds timeout>]" << endl;
						exit(-1);
				}
			}
		}
	} else if(argc > 3){
		cout << "Syntax: " << argv[0] << " [-t <milliseconds timeout>]" << endl;
		exit(-1);
	}

	test_bench tb("testbench", argv[0]);
	auto then = chrono::high_resolution_clock::now();
	
	if(timeout != -1)
		sc_start(timeout, SC_MS);
	else
		sc_start();

	auto now = chrono::high_resolution_clock::now();
	auto diff = now - then;
	cout << endl << "Simulation time: " << (float) ((tb.MPSoC->pe[0]->tick_counter.read() * 10.0f) / 1000.0f / 1000.0f) << "ms" << endl;
	cout << "Wall time: " << chrono::duration_cast<chrono::duration<double>>(diff).count() << "s" << endl;
  	return 0;
}
#endif
