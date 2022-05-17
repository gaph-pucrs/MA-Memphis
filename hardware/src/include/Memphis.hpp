/**
 * MA-Memphis
 * @file Memphis.hpp
 * 
 * @author Unknown
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date 2016
 * 
 * @brief Connect all PEs
 */

#pragma once

#include <systemc>

#include "PE.hpp"
#include "standards.h"
#include "definitions.h"

SC_MODULE(Memphis){
	sc_in<bool>		clock;
	sc_in<bool>		reset;

	/* AppInjector interface */
	sc_out<bool>		memphis_app_injector_tx;
	sc_in<bool>			memphis_app_injector_credit_i;
	sc_out<regflit>		memphis_app_injector_data_out;

	sc_in<bool>			memphis_app_injector_rx;
	sc_out<bool>		memphis_app_injector_credit_o;
	sc_in<regflit>		memphis_app_injector_data_in;

	/* MAInjector interface */
	sc_out<bool>		memphis_ma_injector_tx;
	sc_in<bool>			memphis_ma_injector_credit_i;
	sc_out<regflit>		memphis_ma_injector_data_out;

	sc_in<bool>			memphis_ma_injector_rx;
	sc_out<bool>		memphis_ma_injector_credit_o;
	sc_in<regflit>		memphis_ma_injector_data_in;

	/* IO interface - Create the IO interface for your component here: */
	
	PE  *	pe[N_PE];//store slaves PEs

	SC_HAS_PROCESS(Memphis);
	Memphis(sc_module_name name_, std::string path);

private:
	/* NoC interconnection */
	sc_signal<bool>		tx[N_PE][NPORT-1];
	sc_signal<regflit>	data_out[N_PE][NPORT-1];
	sc_signal<bool>		credit_i[N_PE][NPORT-1];
	
	sc_signal<bool> 	rx[N_PE][NPORT-1];
	sc_signal<regflit>	data_in[N_PE][NPORT-1];
	sc_signal<bool>		credit_o[N_PE][NPORT-1];

	/* BrNoC interconnection */
	sc_signal<bool>		br_req_in[N_PE][NPORT - 1];
	sc_signal<bool>		br_ack_in[N_PE][NPORT - 1];
	sc_signal<uint32_t>	br_payload_in[N_PE][NPORT - 1];
	sc_signal<uint32_t>	br_address_in[N_PE][NPORT - 1];
	sc_signal<uint16_t>	br_producer_in[N_PE][NPORT - 1];
	sc_signal<uint8_t>	br_id_svc_in[N_PE][NPORT - 1];
	sc_signal<uint8_t>	br_ksvc_in[N_PE][NPORT - 1];

	sc_signal<bool>		br_req_out[N_PE][NPORT - 1];
	sc_signal<bool>		br_ack_out[N_PE][NPORT - 1];
	sc_signal<uint32_t>	br_payload_out[N_PE][NPORT - 1];
	sc_signal<uint32_t>	br_address_out[N_PE][NPORT - 1];
	sc_signal<uint16_t>	br_producer_out[N_PE][NPORT - 1];
	sc_signal<uint8_t>	br_id_svc_out[N_PE][NPORT - 1];
	sc_signal<uint8_t>	br_ksvc_out[N_PE][NPORT - 1];

	static regaddress RouterAddress(int router);

	void pes_interconnection();
	void br_interconnection();
};
