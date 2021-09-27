#include <iostream>

#include "BrLiteControl.hpp"

using namespace sc_core;
using namespace std;

BrLiteControl::BrLiteControl(sc_module_name _name, uint16_t _address):
	sc_module(_name),
	address(_address),
	id(0)
{
	SC_METHOD(config);
	sensitive << reset;
	sensitive << payload_cfg;
	sensitive << address_cfg;
	sensitive << producer_cfg;
	sensitive << id_svc_cfg;
	sensitive << start_cfg;
	sensitive << ack_in;
}

void BrLiteControl::config()
{
	if(reset){
		id = 0;
		req_out = false;
		start = false;
		return;
	}

	if(payload_cfg){
		payload_out = data_in;
		// cout << "Payload is set to " << hex << data_in << endl;
	} else if(address_cfg){
		uint32_t address_reg = 0;
		address_reg |= (address << 16);
		address_reg |= (data_in & 0xFFFF);
		address_out = address_reg;
		// cout << "Address is set to " << hex << address_reg << endl;
	} else if(producer_cfg){
		uint16_t prod_id = data_in;
		producer_out = prod_id;
		// cout << "Producer is set to " << hex << prod_id << endl;
	} else if(id_svc_cfg){
		uint8_t id_svc = 0;
		id_svc |= id << 3;
		id = (id + 1) % 0x1F;
		id_svc |= (data_in & 0x7);
		id_svc_out = id_svc;
		// cout << "ID/Service is set to " << hex << (int)id_svc << endl;
	} else if(start_cfg){
		// cout << "Starting send BrNoC" << endl;
		start = true;
	}

	if(start && !ack_in){
		req_out = true;
		start = false;
	}

	if(ack_in){
		// cout << "Ending send BrNoC" << endl;
		req_out = false;
	}
}
