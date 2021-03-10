/*
 *  MAInjector.hpp
 *
 *  Created on: 4 de jan de 2021
 *      Author: ???
 */

#pragma once

#include <systemc.h>
#include <vector>
#include <string>
// #include <iostream>
// #include <map>

#include "../standards.h"

// using namespace std;

// #define TAM_FLIT 				32 	//Size of the Packet-Swtiching NoC flit
#define CONSTANT_PACKET_SIZE	13 	//Constant ServiceHeader packet size (more info inside software/modules/packet.h)
// #define MPE_ADDR				0 	//PE address of the manager PE
// #define TASK_NUMBER_INDEX		8 	//Index where is the app task number information within packet APP_REQ_ACK
// #define TASK_DESCRIPTOR_SIZE	6	//6 is number of lines to represent a task description. Keeps this number equal to build_env/scripts/app_builder.py
// #define MAN_APP_DESCRIPTOR_SIZE	4 	//This number represents the number of lines that MAN_app has into the file my_scenario/appstart.txt. If you include a new MAN_app task, please increase this value in +1


// typedef sc_uint<TAM_FLIT > regflit;

// //Services
#define 	MESSAGE_REQUEST 				0x00000010
#define		MESSAGE_DELIVERY				0x00000020
#define 	DATA_AV							0x00000310
#define 	TASK_ALLOCATION     			0x00000040
#define		MA_ALLOCATION					0x00000151
#define 	APP_ALLOCATION_REQUEST			0x00000240
#define		APP_MAPPING_COMPLETE			0x00000440

#define		MA_INJECTOR_ADDRESS (0x80000000 | (io_port[MAINJECTOR] << 29) | ((MAINJECTOR / N_PE_X) << 8) | (MAINJECTOR % N_PE_X))

SC_MODULE(MAInjector){
public:
	/* Ports used by the many-core */
	sc_in<bool>	clock;
	sc_in<bool>	reset;

	sc_in<bool>		rx;
	sc_in<regflit>	data_in;
	sc_out<bool>	credit_out;

	sc_out<bool>	tx;
	sc_out<regflit>	data_out;
	sc_in<bool>		credit_in;

	SC_HAS_PROCESS(MAInjector);
	MAInjector(sc_module_name _name);

private:
	enum MA_BOOT_FSM {
		BOOT_INIT,
		BOOT_WAIT,
		BOOT_TASKS,
		BOOT_MAP,
		BOOT_SEND,
		BOOT_CONFIRM,
		BOOT_FINISHED
	};
	enum SEND_PKT_FSM {
		SEND_IDLE,
		WAIT_MSG_REQ,
		SEND_DATA_AV,
		SEND_MSG_REQ,
		SEND_PKT,
		WAIT_CREDIT,
		SEND_FINISHED
	};
	enum RCV_PKT_FSM {
		RCV_HEADER,
		RCV_PAYLOAD_SIZE,
		RCV_SERVICE,
		RCV_DATA_AV,
		RCV_MSG_REQ,
		RCV_MSG_DLVR,
		WAIT_SEND_REQ,
		WAIT_SEND_DLVR,
		WAIT_ALLOCATION,
		CHECK_NEXT_MAP,
		RCV_MAP_COMPLETE
	};

	sc_signal<bool> sig_credit_out;

	enum MA_BOOT_FSM ma_boot_state;
	enum SEND_PKT_FSM send_pkt_state;
	enum RCV_PKT_FSM rcv_pkt_state;

	std::vector<std::pair<std::string, int> > tasks;
	std::vector<unsigned int> packet;
	std::vector<unsigned int> rcv_packet;

	unsigned long tick_cnt;

	void task_load(std::string task, int id, int address, int mapper_id, int mapper_address);
	void ma_load();

	void timer();
	void credit_out_update();

	void ma_boot();
	void send_packet();
	void rcv_packet();
};

// private:
// 	//Functions;
// 	void app_descriptor_loader();
// 	void task_allocation_loader(unsigned int, unsigned int, unsigned int, unsigned int);
// 	void app_mapping_loader();

// 	//Sequential logic
// 	void bootloader();
// 	void monitor_new_app();
// 	void send_packet();
// 	void receive_packet();

// 	//Combinational logic
// 	

// 	//FSM
// 	enum FSM_bootloader{INITIALIZE, WAIT_SEND_BOOT, BOOTLOADER_FINISHED};
// 	enum FSM_new_app_monitor{IDLE_MONITOR, MONITORING, WAITING_TIME, WAITING_SEND_NEW_APP};

// 	enum FSM_bootloader 		EA_bootloader;
// 	enum FSM_new_app_monitor 	EA_new_app_monitor;
// 	enum FSM_send_packet 		EA_send_packet;
// 	enum FSM_receive_packet 	EA_receive_packet;

// 	unsigned int current_time;

// 	//Line counter, used to wakl over app_start
// 	unsigned int line_counter;

// 	//Appstart info
// 	unsigned int req_app_start_time;
// 	unsigned int req_app_task_number;
// 	string 		 req_app_name;
// 	int 		 req_app_cluster_id;
// 	int * task_static_mapping;

// 	//Used inside EA_receive_packet
// 	unsigned int cluster_address;
// 	unsigned int ack_app_id;
// 	unsigned int payload_size;
// 	unsigned int flit_counter;
// 	unsigned int req_task_id;
// 	unsigned int req_task_allocated_proc;
// 	unsigned int req_task_master_ID;
// 	unsigned int req_task_id_real;

// 	//Used inside EA_send_packet
// 	unsigned int packet_size;
// 	unsigned int * packet;
// 	map<int, int> pending_allocation;
// 	map<int, int>::iterator pending_it;

// };
