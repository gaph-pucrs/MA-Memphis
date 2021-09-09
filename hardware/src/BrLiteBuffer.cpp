#include <iostream>

#include "BrLiteBuffer.hpp"

using namespace sc_core;
using namespace std;

BrLiteBuffer::BrLiteBuffer(sc_module_name _name) :
	sc_module(_name)
{
	SC_METHOD(buffer_in);
	sensitive << reset;
	sensitive << req_in;

}

void BrLiteBuffer::buffer_in()
{
	if(reset){
		ack_out = false;
		return;
	}

	if(req_in.event()){
		if(req_in){
			// cout << "Received payload " << hex << payload_in << " from " << (address_in >> 16) << endl;
			ack_out = true;
		} else {
			ack_out = false;
		}
	}
}
