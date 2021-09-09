#pragma once

#include <systemc>

SC_MODULE(BrLiteBuffer){
public:
	sc_core::sc_in<bool>		clock;
	sc_core::sc_in<bool>		reset;

	sc_core::sc_in<bool>		req_in;
	sc_core::sc_out<bool>		ack_out;
	sc_core::sc_in<uint32_t>	payload_in;
	sc_core::sc_in<uint32_t>	address_in;
	sc_core::sc_in<uint8_t>		id_svc_in;
	
	SC_HAS_PROCESS(BrLiteBuffer);
	BrLiteBuffer(sc_core::sc_module_name _name);

private:
	void buffer_in();

};
