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

	sc_core::sc_out<bool>		empty;
	sc_core::sc_in<bool>		read_in;
	sc_core::sc_out<uint32_t>	payload_out;
	
	SC_HAS_PROCESS(BrLiteBuffer);
	BrLiteBuffer(sc_core::sc_module_name _name);

private:
	static const uint8_t BR_BUFFER_SIZE = 8;

	typedef struct _buffer {
		sc_core::sc_signal<uint32_t> payload;
	} buffer_t;

	sc_core::sc_signal<uint8_t> head;

	buffer_t buffer[BR_BUFFER_SIZE];
	uint8_t tail;
	bool full;

	void buffer_in();
	void data_out();
};
