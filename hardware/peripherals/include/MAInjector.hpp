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

#include "standards.h"

/* Packet size. Check software/kernel/modules/include/packet.h. */
#define CONSTANT_PACKET_SIZE	13
#define TASK_DESCRIPTOR_SIZE	7

/* Platform kernel/management services */
#define MESSAGE_REQUEST			0x00000010
#define MESSAGE_DELIVERY		0x00000020
#define DATA_AV					0x00000310
#define TASK_ALLOCATION			0x00000040
#define NEW_APP					0x00000150
#define APP_ALLOCATION_REQUEST	0x00000240
#define APP_MAPPING_COMPLETE	0x00000440

#define MA_INJECTOR_ADDRESS (0x80000000 | (io_port[MAINJECTOR] << 29) | ((MAINJECTOR / N_PE_X) << 8) | (MAINJECTOR % N_PE_X))

SC_MODULE(MAInjector){
public:
	/* Ports used by the many-core */
	sc_in<bool>		clock;
	sc_in<bool>		reset;

	sc_in<bool>		rx;
	sc_in<regflit>	data_in;
	sc_out<bool>	credit_out;

	sc_out<bool>	tx;
	sc_out<regflit>	data_out;
	sc_in<bool>		credit_in;

	SC_HAS_PROCESS(MAInjector);
	MAInjector(sc_module_name _name, std::string path_);

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
		RCV_MAP_COMPLETE
	};

	sc_signal<bool> sig_credit_out;

	enum MA_BOOT_FSM ma_boot_state;
	enum SEND_PKT_FSM send_pkt_state;
	enum RCV_PKT_FSM rcv_pkt_state;

	std::vector<std::pair<std::string, int> > tasks;
	std::vector<unsigned int> packet;
	std::vector<unsigned int> packet_in;

	unsigned long tick_cnt;
	std::string path;

	void task_load(std::string task, int id, int address, int mapper_id, int mapper_address);
	void ma_load();

	void timer();
	void credit_out_update();

	void ma_boot();
	void send_packet();
	void rcv_packet();
};
