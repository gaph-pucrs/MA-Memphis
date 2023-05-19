#include "FlitSniffer.hpp"

using namespace sc_core;
using namespace std;

FlitSniffer::FlitSniffer(sc_module_name _name, regaddress _address, int port, string _path) : 
	sc_module(_name),
	log(_path + "/flit_sniffer/" + to_string((_address >> 8) & 0xFF) + "x" + to_string(_address & 0xFF) + "_" + to_string(port) + ".log")
{
	SC_METHOD(monitor);
	sensitive << clock_i.pos();
}

void FlitSniffer::monitor()
{
	if(reset_i){
		log.clear();
	} else if(rx_i && credit_o_i) {
		log << dec << timer_i << " " << hex << data_i << endl;
	}
}
