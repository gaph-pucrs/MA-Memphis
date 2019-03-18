/*** RAM memory initialized with kernel object code ***/

#include <systemc.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../../standards.h"
using namespace std;

#define RAM_SIZE	MEMORY_SIZE_BYTES/4

SC_MODULE(ram) {

	sc_in< bool >			clk;
	sc_in< sc_uint<30> >	address_a;
	sc_in< bool >			enable_a;
	sc_in < sc_uint<4> >	wbe_a;
	sc_in < sc_uint<32> >	data_write_a;
	sc_out < sc_uint<32> >	data_read_a;

	sc_in< sc_uint<30> >	address_b;
	sc_in< bool >			enable_b;
	sc_in < sc_uint<4> >	wbe_b;
	sc_in < sc_uint<32> >	data_write_b;
	sc_out < sc_uint<32> >	data_read_b;

	unsigned long ram_data[RAM_SIZE];
	unsigned long byte[4];
	unsigned long half_word[2];

	/*** Process functions ***/
	void read_a();
	void write_a();

	void read_b();
	void write_b();

	void load_ram();

	SC_HAS_PROCESS(ram);
	ram(sc_module_name name_, unsigned int address_ = 0) : sc_module(name_), router_address(address_) {

		load_ram();

		SC_METHOD(read_a);
		sensitive << clk.pos();

		SC_METHOD(write_a);
		sensitive << clk.pos();

		SC_METHOD(read_b);
		sensitive << clk.pos();

		SC_METHOD(write_b);
		sensitive << clk.pos();

		// Byte masks.
		byte[0] = 0x000000FF;
		byte[1] = 0x0000FF00;
		byte[2] = 0x00FF0000;
		byte[3] = 0xFF000000;

		// Half word masks.
		half_word[0] = 0x0000FFFF;
		half_word[1] = 0xFFFF0000;

		// Initializes RAM memory with kernel object code.
	}

	private:
		unsigned int router_address;


};
