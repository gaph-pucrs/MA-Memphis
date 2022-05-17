#pragma once

#include <systemc>

#include "definitions.h"

SC_MODULE(BrLiteControl){
public:
	sc_core::sc_in<bool>		clock;
	sc_core::sc_in<bool>		reset;

	sc_core::sc_in<bool>		payload_cfg;
	sc_core::sc_in<bool>		address_cfg;
	sc_core::sc_in<bool>		producer_cfg;
	sc_core::sc_in<bool>		id_svc_cfg;
	sc_core::sc_in<bool>		ksvc_cfg;
	sc_core::sc_in<bool>		start_cfg;
	sc_core::sc_in<uint32_t>	data_in;

	sc_core::sc_out<uint32_t>	payload_out;
	sc_core::sc_out<uint32_t>	address_out;
	sc_core::sc_out<uint16_t>	producer_out;
	sc_core::sc_out<uint8_t>	id_svc_out;
	sc_core::sc_out<uint8_t>	ksvc_out;
	sc_core::sc_out<bool>		req_out;
	sc_core::sc_in<bool>		ack_in;

	SC_HAS_PROCESS(BrLiteControl);
	BrLiteControl(sc_core::sc_module_name _name, uint16_t _address);

private:
	uint16_t address;
	uint8_t id;
	bool start;

	void config();

};
