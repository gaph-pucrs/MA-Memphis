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
	}
}

void BrLiteBuffer::buffer_in()
{
	if(reset){
		ack_out = false;
		empty = true;
		head = 0;
		tail = 0;
		return;
	}

	/* Priority: read */
	if(read_in && !empty){
		// cout << "Received READ_IN to head " << (int)head << " with payload " << buffer[head].payload << endl;
		uint8_t next_head = (head + 1) % BR_BUFFER_SIZE;
		head = next_head;
		if(next_head == tail){
			empty = true;
		}
	} else if(req_in && !ack_out){
		buffer[tail].payload = payload_in;
		uint8_t next_head = head;
		if(tail == head && !empty){	/* Allow overwriting */
			next_head = (head + 1) % BR_BUFFER_SIZE;
			head = next_head;
		}
		tail = (tail + 1) % BR_BUFFER_SIZE;
		empty = false;
		ack_out = true;
		// cout << "Received payload " << hex << payload_in << " from " << (address_in >> 16) << ". HEAD: " << (int)next_head << " TAIL: " << (int)tail << endl;
	} 
	
	if(!req_in){
		ack_out = false;
	}
}

void BrLiteBuffer::data_out()
{
	payload_out = buffer[head].payload;
}
