#pragma once

#include <fstream>

#include <systemc>

#include "standards.h"

SC_MODULE(FlitSniffer) {
public:
	sc_core::sc_in<bool>		clock_i;
	sc_core::sc_in<bool>		reset_i;
	sc_core::sc_in<sc_uint<32>>	timer_i;

	sc_core::sc_in<bool>		rx_i;
	sc_core::sc_in<bool>		credit_o_i;
	sc_core::sc_in<regflit>		data_i;

	SC_HAS_PROCESS(FlitSniffer);
	FlitSniffer(sc_core::sc_module_name _name, regaddress _address, int port, std::string _path);

private:
	std::ofstream		log;

	void monitor();

};
