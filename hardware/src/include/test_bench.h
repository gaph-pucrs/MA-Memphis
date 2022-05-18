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

using namespace std;

#include "Memphis.hpp"
#include "AppInjector.hpp"
#include "MAInjector.hpp"
#include "definitions.h"

SC_MODULE(test_bench) {
	
	sc_signal< bool >	clock;
	sc_signal< bool >	reset;
		
	//IO signals connecting App Injector and Memphis
	sc_signal<bool>		memphis_injector_tx;
	sc_signal<bool>		memphis_injector_credit_i;
	sc_signal<regflit> 	memphis_injector_data_out;
	sc_signal<bool>		memphis_injector_rx;
	sc_signal<bool>		memphis_injector_credit_o;
	sc_signal<regflit>	memphis_injector_data_in;

	//Create the signals of your IO component here:
	sc_signal<bool>		memphis_mai_tx;
	sc_signal<bool>		memphis_mai_credit_i;
	sc_signal<regflit> 	memphis_mai_data_out;
	sc_signal<bool>		memphis_mai_rx;
	sc_signal<bool>		memphis_mai_credit_o;
	sc_signal<regflit>	memphis_mai_data_in;


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
	test_bench(sc_module_name name_, const char *filename_= "output_master.txt", std::string program_path = ".") :
    sc_module(name_), filename(filename_)
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
	private:
		const char *filename;
};


#ifndef MTI_SYSTEMC

int sc_main(int argc, char *argv[]){
	int time_to_run=0;
	int i;
	const char *filename = "output_master.txt";
	
	if(argc<3){
		cout << "Sintax: " << argv[0] << " -c <milisecons to execute> [-o <output filename>]" << endl;
		exit(EXIT_FAILURE);
	}
	
	for (i = 1; i < argc; i++){/* Check for a switch (leading "-"). */
		if (argv[i][0] == '-') {/* Use the next character to decide what to do. */
			switch (argv[i][1]){
				case 'c':
					time_to_run = atoi(argv[++i]);
				break;
				case 'o':
					filename = argv[++i];
					cout << filename << endl;
				break;
				default:
					cout << "Sintax: " << argv[0] << "-c <milisecons to execute> [-o <output name file>]" << endl;
					exit(EXIT_FAILURE);
				break;
			}
		}
	}
	
	
	test_bench tb("testbench",filename, argv[0]);
	sc_start(time_to_run,SC_MS);
  	return 0;
}
#endif
