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
	sc_core::sc_in<uint16_t>	producer_in;
	sc_core::sc_in<uint8_t>		ksvc_in;

	sc_core::sc_out<bool>		empty;
	sc_core::sc_in<bool>		read_in;
	sc_core::sc_in<bool>		pop_sig;
	sc_core::sc_out<uint32_t>	payload_out;
	sc_core::sc_out<uint32_t>	producer_out;
	sc_core::sc_out<uint8_t>	ksvc_out;
	
	SC_HAS_PROCESS(BrLiteBuffer);
	BrLiteBuffer(sc_core::sc_module_name _name);

private:
	static const uint8_t BR_BUFFER_SIZE = 8;

	typedef struct _buffer {
		sc_core::sc_signal<uint32_t> payload;
		sc_core::sc_signal<uint32_t> producer;
		sc_core::sc_signal<uint8_t> ksvc;
	} buffer_t;

	sc_core::sc_signal<uint8_t> head;

	buffer_t buffer[BR_BUFFER_SIZE];
	uint8_t tail;
	bool full;

	void buffer_in();
	void data_out();
};
