/*
 * app_injector.h
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro
 */

#pragma once

#include <systemc.h>
#include <string>
#include <vector>
#include <map>

#include "standards.h"

/* Packet size. Check software/kernel/modules/include/packet.h. */
#define CONSTANT_PACKET_SIZE	13
/* Number of lines to represent a task description. Check build_env/scripts/app_builder.py */
#define TASK_DESCRIPTOR_SIZE	4
// #define MPE_ADDR				0 	//PE address of the manager PE
// #define TASK_NUMBER_INDEX		8 	//Index where is the app task number information within packet APP_REQ_ACK

/* Platform kernel/management services */
#define MESSAGE_REQUEST			0x00000010
#define MESSAGE_DELIVERY		0x00000020
#define DATA_AV					0x00000310
#define TASK_ALLOCATION			0x00000040
#define NEW_APP					0x00000150
#define APP_ALLOCATION_REQUEST	0x00000240
#define APP_MAPPING_COMPLETE	0x00000440

#define APP_INJECTOR_ADDRESS (0x80000000 | (io_port[APP_INJECTOR] << 29) | ((APP_INJECTOR / N_PE_X) << 8) | (APP_INJECTOR % N_PE_X))

SC_MODULE(AppInjector){
public:
	sc_in<bool>		clock;
	sc_in<bool>		reset;

	sc_in<bool>		rx;
	sc_in<regflit>	data_in;
	sc_out<bool>	credit_out;

	sc_out<bool>	tx;
	sc_out<regflit>	data_out;
	sc_in<bool>		credit_in;

	SC_HAS_PROCESS(AppInjector);
	AppInjector(sc_module_name _name, std::string _path);

private:
	enum SEND_PKT_FSM {
		SEND_IDLE, 
		SEND_WAIT_REQUEST, 
		SEND_DATA_AV, 
		SEND_MSG_REQUEST, 
		SEND_PACKET, 
		SEND_WAIT_CREDIT, 
		SEND_FINISHED
	};
	enum RCV_PKT_FSM {
		RCV_HEADER, 
		RCV_PAYLOAD_SIZE, 
		RCV_SERVICE, 
		RECEIVE_DATA_AV, 
		RECEIVE_MESSAGE_REQUEST, 
		RECEIVE_MESSAGE_DELIVERY, 
		RCV_WAIT_REQ, 
		RCV_WAIT_DLVR, 
		RCV_WAIT_ALLOCATION, 
		RECEIVE_MAPPING_COMPLETE
	};
	enum MONITOR_FSM {
		MONITOR_IDLE, 
		MONITOR_ACTIVE, 
		MONITOR_WAIT_TIME, 
		MONITOR_SEND_NEW_APP,
		MONITOR_SEND_TASK,
		MONITOR_MAP
	};

	sc_signal<bool> sig_credit_out;

	enum MONITOR_FSM	monitor_state;
	enum SEND_PKT_FSM	send_pkt_state;
	enum RCV_PKT_FSM	rcv_pkt_state;

	std::vector<unsigned> packet;
	std::vector<unsigned> packet_in;

	unsigned long tick_cnt;
	std::string path;

	void app_descriptor_loader(std::string name, unsigned task_cnt, std::vector<int>& static_mapping);
	void task_allocation_loader(unsigned id, unsigned addr, unsigned mapper_id, unsigned mapper_addr);
	std::string get_app_repo_path(unsigned app_id);

	void monitor_new_app();
	void send_packet();
	void receive_packet();
	void credit_out_update();
	void timer();
};
