//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  ram.cpp
//
//  Brief description: Reading and writing memory
//
//------------------------------------------------------------------------------------------------

#include "ram.h"

#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(ram);
#endif

void ram::load_ram(){

	string line;
	int i = 0;

	char ram_path[20];
	sprintf(ram_path, "ram_pe/ram%dx%d.txt", (router_address >> 8), (router_address & 0xFF));
	ifstream repo_file (ram_path);

	if (repo_file.is_open()) {
		while ( getline (repo_file,line) ) {

			if (i == RAM_SIZE){
				cout << "ERROR: Ram file "<< ram_path << "is greater than RAM_SIZE = " << RAM_SIZE << endl;
				sc_stop();
			}

			//Converts a hex string to unsigned integer
			sscanf( line.substr(0, 8).c_str(), "%lx", &ram_data[i] );
			i++;

		}
		repo_file.close();
	} else {
		cout << "Unable to open file " << ram_path << endl;
	}
}

/*** Memory read port A ***/
void ram::read_a() {

	unsigned int address;

	address = (unsigned int)address_a.read();

	if ( address < RAM_SIZE )
		data_read_a.write(ram_data[address]);
}


/*** Memory write port A ***/
void ram::write_a() {

	unsigned int data, address;
	unsigned char wbe;

	wbe = (unsigned char)wbe_a.read();
	address = (unsigned int)address_a.read();


	if ( wbe != 0 && address < RAM_SIZE) {
		data = ram_data[address];

		switch(wbe) {
			case 0xF:	// Write word
				ram_data[address] = data_write_a.read();
			break;

			case 0xC:	// Write MSW
				ram_data[address] = (data & ~half_word[1]) | (data_write_a.read() & half_word[1]);
			break;

			case 3:		// Write LSW
				ram_data[address] = (data & ~half_word[0]) | (data_write_a.read() & half_word[0]);
			break;

			case 8:		// Write byte 3
				ram_data[address] = (data & ~byte[3]) | (data_write_a.read() & byte[3]);
			break;

			case 4:		// Write byte 2
				ram_data[address] = (data & ~byte[2]) | (data_write_a.read() & byte[2]);
			break;

			case 2:		// Write byte 1
				ram_data[address] = (data & ~byte[1]) | (data_write_a.read() & byte[1]);
			break;

			case 1:		// Write byte 0
				ram_data[address] = (data & ~byte[0]) | (data_write_a.read() & byte[0]);
			break;
		}
	}
}


/*** Memory read port B ***/
void ram::read_b() {

	unsigned int address;

	address = (unsigned int)address_b.read();

	if ( address < RAM_SIZE )
		data_read_b.write(ram_data[address]);
}


/*** Memory write port B ***/
void ram::write_b() {

	unsigned int data, address;
	unsigned char wbe;

	wbe = (unsigned char)wbe_b.read();
	address = (unsigned int)address_b.read();


	if ( wbe != 0 && address < RAM_SIZE) {
		data = ram_data[address];

		switch(wbe) {
			case 0xF:	// Write word
				ram_data[address] = data_write_b.read();
			break;

			case 0xC:	// Write MSW
				ram_data[address] = (data & ~half_word[1]) | (data_write_b.read() & half_word[1]);
			break;

			case 3:		// Write LSW
				ram_data[address] = (data & ~half_word[0]) | (data_write_b.read() & half_word[0]);
			break;

			case 8:		// Write byte 3
				ram_data[address] = (data & ~byte[3]) | (data_write_b.read() & byte[3]);
			break;

			case 4:		// Write byte 2
				ram_data[address] = (data & ~byte[2]) | (data_write_b.read() & byte[2]);
			break;

			case 2:		// Write byte 1
				ram_data[address] = (data & ~byte[1]) | (data_write_b.read() & byte[1]);
			break;

			case 1:		// Write byte 0
				ram_data[address] = (data & ~byte[0]) | (data_write_b.read() & byte[0]);
			break;
		}
	}
}

