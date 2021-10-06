#include <iostream>

#include "BrLiteBuffer.hpp"

using namespace sc_core;
using namespace std;

BrLiteBuffer::BrLiteBuffer(sc_module_name _name) :
	sc_module(_name)
{
	SC_METHOD(buffer_in);
	sensitive << clock.pos();

	SC_METHOD(data_out);
	sensitive << head;
	for(int i = 0; i < BR_BUFFER_SIZE; i++){
		sensitive << buffer[i].payload;
		sensitive << buffer[i].producer;
		sensitive << buffer[i].ksvc;
	}
}

void BrLiteBuffer::buffer_in()
{
	if(reset){
		ack_out = false;
		empty = true;
		full = false;
		head = 0;
		tail = 0;
		return;
	}

	/* Priority: read */
	if(read_in && !empty){
		// cout << "Received READ_IN to head " << (int)head << " with payload " << buffer[head].payload << endl;
		uint8_t next_head = (head + 1) % BR_BUFFER_SIZE;
		if(next_head == tail)
			empty = true;
		head = next_head;
		full = false;
	} else if(req_in && !full && !ack_out){
		buffer[tail].payload = payload_in;
		buffer[tail].ksvc = ksvc_in;

		uint32_t producer = 0;
		producer |= address_in & 0xFFFF0000;
		producer |= producer_in;
		buffer[tail].producer = producer;

		uint8_t next_tail = (tail + 1) % BR_BUFFER_SIZE;
		if(next_tail == head)	
			full = true;	/* Disallow overwriting */
		tail = next_tail;
		empty = false;

		ack_out = true;
		// cout << "Received payload " << hex << payload_in << " from " << (address_in >> 16) << ". HEAD: " << (int)head << " TAIL: " << (int)tail << endl;
	} 
	
	if(!req_in){
		ack_out = false;
	}
}

void BrLiteBuffer::data_out()
{
	payload_out = buffer[head].payload;
	producer_out = buffer[head].producer;
	ksvc_out = buffer[head].ksvc;
}
