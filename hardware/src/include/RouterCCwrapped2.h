#ifndef _SCGENMOD_routerCCWrapped_
#define _SCGENMOD_routerCCWrapped_

#include <systemc.h>

#include "standards.h"
#include "RouterCCwrapped.h"

SC_MODULE(RouterCCwrapped) {
		sc_in<bool> clock;
		sc_in<bool> reset;
		sc_in<bool > rx[NPORT];
		sc_in<regflit > data_in[NPORT];
		sc_out<bool > credit_o[NPORT];
		sc_out<bool > tx[NPORT];
		sc_out<regflit > data_out[NPORT];
		sc_in<bool > credit_i[NPORT];

		//signals to bind in wrapped module
		sc_signal<regNport > rx_internal;
		sc_signal<regNport > credit_o_internal;
		sc_signal<regNport > tx_internal;
		sc_signal<regNport > credit_i_internal;

		RouterCC *router;

		void upd_rx();
		void upd_credit_o();
		void upd_tx();
		void upd_credit_i();

		//Traffic monitor
		sc_in<sc_uint<32 > > tick_counter;
		unsigned char SM_traffic_monitor[NPORT];
		unsigned int target_router[NPORT];
		unsigned int header_time[NPORT];
		unsigned short bandwidth_allocation[NPORT];
		unsigned short payload[NPORT];
		unsigned short payload_counter[NPORT];
		unsigned int service[NPORT];
		unsigned int task_id[NPORT];
		unsigned int counter_target[NPORT];
		unsigned int consumer_id[NPORT];
		unsigned int SR_found[NPORT];
		void traffic_monitor();

		int i;


		SC_HAS_PROCESS(RouterCCwrapped);
		RouterCCwrapped(sc_module_name name_, regaddress address_ = 0x0000, std::string _path = "") :
		sc_module(name_), address(address_), path(_path)
		{

			const char* generic_list[1];
			generic_list[0] = strdup("address=x\"AAAA\"");
			sprintf((char*) generic_list[0],"address=x\"%.4x\"",(int)address);

			router = new RouterCC("RouterCC", "RouterCC", 1, generic_list);

			router->reset(reset);
			router->clock(clock);
			router->rx(rx_internal);
			//ports interconnection are mixed due to an issue in SystemC - VHDL integration
			router->data_in[LOCAL](data_in[EAST]);
			router->data_in[SOUTH](data_in[WEST]);
			router->data_in[NORTH](data_in[NORTH]);
			router->data_in[WEST](data_in[SOUTH]);
			router->data_in[EAST](data_in[LOCAL]);
			router->credit_o(credit_o_internal);
			router->tx(tx_internal);
			//ports interconnection are mixed due to an issue in SystemC - VHDL integration
			router->data_out[LOCAL](data_out[EAST]);
			router->data_out[SOUTH](data_out[WEST]);
			router->data_out[NORTH](data_out[NORTH]);
			router->data_out[WEST](data_out[SOUTH]);
			router->data_out[EAST](data_out[LOCAL]);
			router->credit_i(credit_i_internal);			

			// Cleanup the memory allocated for the generic list
			for (i = 0; i < 1; i++)
				free((char*)generic_list[i]);


			SC_METHOD(upd_rx);
			for (i = 0; i < NPORT; i++){
				sensitive << rx[i];
			}

			SC_METHOD(upd_credit_i);
			for (i = 0; i < NPORT; i++){
				sensitive << credit_i[i];
			}

			//output

			SC_METHOD(upd_credit_o);
			sensitive << credit_o_internal;

			SC_METHOD(upd_tx);
			sensitive << tx_internal;

			SC_METHOD(traffic_monitor);
			sensitive << clock;
			sensitive << reset;
// 
		}
		~RouterCCwrapped()
		{}

	public:
		regaddress address;
		std::string path;

};

#endif

