/*
 * app_injector.h
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro
 */


#ifndef PERIPHERALS_APP_INJECTOR_H_
#define PERIPHERALS_APP_INJECTOR_H_


#include <systemc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;

#define TAM_FLIT 				32 	//Size of the Packet-Swtiching NoC flit
#define CONSTANT_PACKET_SIZE	13 	//Constant ServiceHeader packet size (more info inside software/modules/packet.h)
#define MPE_ADDR				0 	//PE address of the manager PE
#define TASK_NUMBER_INDEX		8 	//Index where is the app task number information within packet APP_REQ_ACK
#define TASK_DESCRIPTOR_SIZE	6	//6 is number of lines to represent a task description. Keeps this number equal to build_env/scripts/app_builder.py
#define MAN_APP_DESCRIPTOR_SIZE	8 	//This number represents the number of lines that MAN_app has into the file my_scenario/appstart.txt. If you include a new MAN_app task, please increase this value in +1


typedef sc_uint<TAM_FLIT > regflit;

//Services
#define 	MESSAGE_REQUEST 				0x00000010
#define		MESSAGE_DELIVERY				0x00000020
#define 	DATA_AV							0x00000310
#define 	TASK_ALLOCATION     			0x00000040
#define		NEW_APP							0x00000150 //Injector to Mestre (carries App descriptor)
#define 	APP_ALLOCATION_REQUEST			0x00000240 //Mestre to Injector (carries tasks properties and mapping)
#define		APP_MAPPING_COMPLETE			0x00000440

#define		APP_INJECTOR_ADDRESS (0x80000000 | (io_ports[APP_INJECTOR] << 29) | ((APP_INJECTOR / N_PE_X) << 8) | (APP_INJECTOR % N_PE_X))

SC_MODULE(app_injector){

	//Ports
	sc_in <bool > 		clock;
	sc_in <bool > 		reset;

	sc_in <bool > 		rx;
	sc_in<regflit > 	data_in;
	sc_out<bool > 		credit_out;

	sc_out <bool > 		tx;
	sc_out<regflit > 	data_out;
	sc_in<bool > 		credit_in;

	//Internal variables/signals
	sc_signal<bool > 	sig_credit_out;

	//Functions;
	void app_descriptor_loader();
	void task_allocation_loader(unsigned int, unsigned int, unsigned int, unsigned int);
	string get_app_repo_path(unsigned int);
	void app_mapping_loader();

	//Sequential logic
	void bootloader();
	void monitor_new_app();
	void send_packet();
	void receive_packet();

	//Combinational logic
	void credit_out_update();

	//FSM
	enum FSM_bootloader{INITIALIZE, WAIT_SEND_BOOT, BOOTLOADER_FINISHED};
	enum FSM_send_packet{IDLE, WAIT_MESSAGE_REQUEST, SEND_DATA_AV, SEND_MSG_REQUEST, SEND_PACKET, WAITING_CREDIT, SEND_FINISHED};
	enum FSM_receive_packet{HEADER, PAYLOAD_SIZE, SERVICE, RECEIVE_DATA_AV, RECEIVE_MESSAGE_REQUEST, RECEIVE_MESSAGE_DELIVERY, WAIT_SEND_REQUEST, WAIT_SEND_DELIVERY, WAITING_SEND_TASK_ALLOCATION, CHECK_NEXT_MAP, RECEIVE_MAPPING_COMPLETE};
	enum FSM_new_app_monitor{IDLE_MONITOR, MONITORING, WAITING_TIME, WAITING_SEND_NEW_APP};

	enum FSM_bootloader 		EA_bootloader;
	enum FSM_new_app_monitor 	EA_new_app_monitor;
	enum FSM_send_packet 		EA_send_packet;
	enum FSM_receive_packet 	EA_receive_packet;

	unsigned int current_time;

	//Line counter, used to wakl over app_start
	unsigned int line_counter;

	//Appstart info
	unsigned int req_app_start_time;
	unsigned int req_app_task_number;
	string 		 req_app_name;
	int 		 req_app_cluster_id;
	int * task_static_mapping;

	//Used inside EA_receive_packet
	unsigned int cluster_address;
	unsigned int ack_app_id;
	unsigned int payload_size;
	unsigned int flit_counter;
	unsigned int req_task_id;
	unsigned int req_task_allocated_proc;
	unsigned int req_task_master_ID;
	unsigned int req_task_id_real;

	//Used inside EA_send_packet
	unsigned int packet_size;
	unsigned int * packet;
	map<int, int> pending_allocation;
	map<int, int>::iterator pending_it;

	SC_HAS_PROCESS(app_injector);
	app_injector (sc_module_name name_) : sc_module(name_) {

		//Variable initialization
		current_time = 0;
		line_counter = 0;
		packet = 0;
		packet_size = 0;
		req_app_start_time = 0;
		req_app_task_number = 0;
		req_app_cluster_id = 0;
		cluster_address = 0;
		ack_app_id = 0;
		payload_size = 0;
		flit_counter = 0;
		req_task_id = 0;
		req_task_allocated_proc = 0;
		req_task_master_ID = 0;
		task_static_mapping = 0;

		EA_receive_packet = HEADER;
		EA_send_packet = IDLE;
		EA_new_app_monitor = MONITORING;
		EA_bootloader = INITIALIZE;

		SC_METHOD(bootloader);
		sensitive << clock.pos();
		sensitive << reset;

		SC_METHOD(monitor_new_app);
		sensitive << clock.pos();
		sensitive << reset;

		SC_METHOD(send_packet);
		sensitive << clock.pos();
		sensitive << reset;

		SC_METHOD(receive_packet);
		sensitive << clock.pos();
		sensitive << reset;

		SC_METHOD(credit_out_update);
		sensitive << sig_credit_out;

	}

};


#endif /* PERIPHERALS_APP_INJECTOR_H_ */
