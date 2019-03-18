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

using namespace std;

#define TAM_FLIT 				32
#define CONSTANT_PACKET_SIZE	13 //Constant ServiceHeader packet size (more info inside software/modules/packet.h)
#define MPE_ADDR				0 //PE address of the manager PE
#define TASK_NUMBER_INDEX		8 //Index where is the app task number information within packet APP_REQ_ACK
#define TASK_DESCRIPTOR_SIZE	6//6 is number of lines to represent a task description. Keeps this number equal to build_env/scripts/app_builder.py


typedef sc_uint<TAM_FLIT > regflit;

//Services
#define 	TASK_ALLOCATION     			0x00000040
#define		NEW_APP_REQ						0x00000290 //Injector to Mestre (carries num_tasks)
#define		APP_REQ_ACK						0x00000300 //Mestre to Injector (carries cluster addr)
#define		NEW_APP							0x00000150 //Injector to Mestre (carries App descriptor)
#define 	APP_ALLOCATION_REQUEST			0x00000240 //Mestre to Injector (carries tasks properties and mapping)

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
	sc_signal<bool > 		sig_credit_out;

	//Functions;
	void app_descriptor_loader();
	void task_allocation_loader();

	//Sequential logic
	void monitor_new_app();
	void send_packet();
	void receive_packet();

	//combinational
	void credit_out_update();

	//FSM
	enum FSM_send_packet{IDLE, SEND_PACKET, WAITING_CREDIT, SEND_FINISHED};
	enum FSM_receive_packet{HEADER, PAYLOAD_SIZE, SERVICE, RECEIVE_APP_ACK, RECEIVE_APP_ALLOCATION, WAITING_SEND_NEW_APP, WAITING_SEND_TASK_ALLOCATION};
	enum FSM_new_app_monitor{MONITORING, WAITING_TIME, WAITING_SEND_APP_REQ, IDLE_MONITOR};

	enum FSM_send_packet EA_send_packet;
	enum FSM_new_app_monitor EA_new_app_monitor;
	enum FSM_receive_packet EA_receive_packet;

	unsigned int current_time;

	//Appstart info
	string app_name;
	unsigned int app_start_time;
	unsigned int app_task_number;
	int app_cluster_id;
	int * task_static_mapping;

	//Used when a app allocation request is received
	unsigned int cluster_address;
	unsigned int app_id;

	//Used when a app allocation request is received
	unsigned int * tasks_info;
	unsigned int task_info_index;
	unsigned int payload_size;
	unsigned int flit_counter;

	//Used to send packets
	unsigned int packet_size;
	unsigned int * packet;


	SC_HAS_PROCESS(app_injector);
	app_injector (sc_module_name name_) : sc_module(name_) {

		//Variable initialization
		current_time = 0;
		packet = 0;
		packet_size = 0;
		app_name = "";
		app_start_time = 0;
		app_task_number = 0;
		app_cluster_id = 0;
		cluster_address = 0;
		tasks_info = 0;
		app_id = 0;
		task_info_index = 0;
		payload_size = 0;
		flit_counter = 0;
		task_static_mapping = 0;
		EA_receive_packet = HEADER;
		EA_send_packet = IDLE;
		EA_new_app_monitor = MONITORING;


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
