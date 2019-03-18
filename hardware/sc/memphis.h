//------------------------------------------------------------------------------------------------
//
//  MEMPHIS -  7.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  2016
//
//  Source name:  memphis.h
//
//  Brief description: to do
//
//------------------------------------------------------------------------------------------------

#include <systemc.h>
#include "standards.h"
#include "pe/pe.h"

#define BL 0
#define BC 1
#define BR 2
#define CL 3
#define CC 4
#define CRX 5
#define TL 6
#define TC 7
#define TR 8



SC_MODULE(memphis) {
	
	sc_in< bool >			clock;
	sc_in< bool >			reset;

	//IO interface - App Injector
	sc_out< bool >			memphis_app_injector_tx;
	sc_in< bool >			memphis_app_injector_credit_i;
	sc_out< regflit >		memphis_app_injector_data_out;

	sc_in< bool >			memphis_app_injector_rx;
	sc_out< bool >			memphis_app_injector_credit_o;
	sc_in< regflit >		memphis_app_injector_data_in;

	//IO interface - Create the IO interface for your component here:



	// Signals to connects the NoC routers
	sc_signal<bool >		tx[N_PE][NPORT-1];
	sc_signal<regflit >		data_out[N_PE][NPORT-1];
	sc_signal<bool >		credit_i[N_PE][NPORT-1];
	
	sc_signal<bool > 		rx[N_PE][NPORT-1];
	sc_signal<regflit >		data_in[N_PE][NPORT-1];
	sc_signal<bool >		credit_o[N_PE][NPORT-1];
		
	pe  *	PE[N_PE];//store slaves PEs
	
	int i,j;
	
	int RouterPosition(int router);
	regaddress RouterAddress(int router);
	regaddress r_addr;
 	void pes_interconnection();
 	
	char pe_name[20];
	int x_addr, y_addr;
	SC_CTOR(memphis){
		
		for (j = 0; j < N_PE; j++) {

			r_addr = RouterAddress(j);
			x_addr = ((int) r_addr) >> 8;
			y_addr = ((int) r_addr) & 0xFF;

			sprintf(pe_name, "PE%dx%d", x_addr, y_addr);
			printf("Creating PE %s\n", pe_name);

			PE[j] = new pe(pe_name, r_addr);
			PE[j]->clock(clock);
			PE[j]->reset(reset);

			for (i = 0; i < NPORT - 1; i++) {
				PE[j]->tx[i](tx[j][i]);
				PE[j]->data_out[i](data_out[j][i]);
				PE[j]->credit_i[i](credit_i[j][i]);
				PE[j]->data_in[i](data_in[j][i]);
				PE[j]->rx[i](rx[j][i]);
				PE[j]->credit_o[i](credit_o[j][i]);
			}
		}

		SC_METHOD(pes_interconnection);
		sensitive << memphis_app_injector_tx;
		sensitive << memphis_app_injector_credit_i;
		sensitive << memphis_app_injector_data_out;
		sensitive << memphis_app_injector_rx;
		sensitive << memphis_app_injector_credit_o;
		sensitive << memphis_app_injector_data_in;
		for (j = 0; j < N_PE; j++) {
			for (i = 0; i < NPORT - 1; i++) {
				sensitive << tx[j][i];
				sensitive << data_out[j][i];
				sensitive << credit_i[j][i];
				sensitive << data_in[j][i];
				sensitive << rx[j][i];
				sensitive << credit_o[j][i];
			}
		}
	}
};

