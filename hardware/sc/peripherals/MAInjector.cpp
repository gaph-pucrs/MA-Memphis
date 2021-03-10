/*
 * app_injector.cpp
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro on GAPH
 *
 *
 *  Description: This injector abstracts a external memory that sends new applications to the many-core system
 */

#include "MAInjector.h"

#include <iostream>
#include <fstream>

/* This line enables the integration with vhdl */
#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(MAInjector);
#endif

MAInjector::MAInjector(sc_module_name _name) :
  sc_module(_name)
{
	ma_boot_state = BOOT_INIT;
	send_pkt_state = SEND_IDLE;
	rcv_pkt_state = RCV_HEADER;

	tick_cnt = 0;

	SC_METHOD(timer);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(ma_boot);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(send_packet);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(rcv_packet);
	sensitive << clock.pos();
	sensitive << reset;


	SC_METHOD(credit_out_update);
	sensitive << sig_credit_out;


// 	//Variable initialization
// 	line_counter = 0;
// 	req_app_start_time = 0;
// 	req_app_task_number = 0;
// 	req_app_cluster_id = 0;
// 	//ack_app_id = 0;
// 	req_task_id = 0;
// 	req_task_allocated_proc = 0;
// 	req_task_master_ID = 0;

// 	EA_new_app_monitor = MONITORING;


// 	SC_METHOD(monitor_new_app);
// 	sensitive << clock.pos();
// 	sensitive << reset;

}

void MAInjector::timer()
{
	if(reset.read())
		tick_cnt = 0;
	else if(clock.posedge())
		tick_cnt++;
}

void MAInjector::credit_out_update()
{
	credit_out.write(sig_credit_out.read());
}

void MAInjector::ma_boot()
{
	static int ma_boot_task_cnt = 0;
	static int sent_task = 0;
	if(reset.read()){
		ma_boot_state = BOOT_INIT;
	} else if(clock.posedge()){
		switch(ma_boot_state){
		case BOOT_INIT:
			std::ifstream ma_boot_info("maboot.txt");
			if(ma_boot_info.is_open()){
				tasks.clear();
				std::string task_name;
				ma_boot_task_cnt = 0;
				while(std::getline(ma_boot_info, task_name)){
					std::string addrstr;
					std::getline(ma_boot_info, addrstr);
					int address = std::stoi(addrstr);

					tasks.pushback(std::makepair(task_name, address));

					std::cout << "Task name: " << task_name << std::endl;
					std::cout << "Address: " << address << std::endl;
					ma_boot_task_cnt++;
				}

				std::cout << "MA boot task count: " << ma_boot_task_cnt << std::endl;

				/* First task should be the mapper directly injected to the system with static mapping */
				if(tasks[0].first.compare("mapper_task") != 0){
					std::cout << "ERROR: first task must be 'mapper_task'" << std::endl;
					ma_boot_state = BOOT_FINISHED;
				}

				if(tasks[0].second == -1){
					std::cout << "ERROR: 'mapper_task' should be static mapped" << std::endl;
					ma_boot_state = BOOT_FINISHED;
				}

				task_load(task[0].first, 0, task[0].second, -1, -1);

				ma_boot_state = BOOT_WAIT;
			} else {
				cout << "Unable to open file mastart.txt" << endl;
			}
			break;
		case BOOT_WAIT:
			/* Waits ends of boot packet transmission */
			if(send_pkt_state == SEND_FINISHED){
				/* Now will request the MA mapping */
				/* All MA startup information is already loaded into 'tasks' structure */
				ma_load();

				ma_boot_state = BOOT_TASKS;
				// std::cout << "bootloader send ok" << std::endl;
			}
			break;
		case BOOT_TASKS:
			if(send_pkt_state == SEND_FINISHED){
				sent_task = 0;
				ma_boot_state = BOOT_MAP;
			}
			break;
		case BOOT_MAP:
			if(rcv_pkt_state == WAIT_ALLOCATION){
				task_load(tasks[rcv_packet[sent_task*2]].first, rcv_packet[sent_task*2], rcv_packet[sent_task*2 + 1], 0, tasks[0].second);
				ma_boot_state = BOOT_SEND;
			}
			break;
		case BOOT_SEND:
			if(send_pkt_state == SEND_FINISHED){
				sent_task++;
				if(sent_task*2 < rcv_packet.size()){
					ma_boot_state = BOOT_MAP;
				} else {
					ma_boot_state = BOOT_CONFIRM;
				}
			}
			break;
		case BOOT_CONFIRM:
			if(rcv_pkt_state == RCV_MAP_COMPLETE){
				ma_boot_state = BOOT_FINISHED;
			}
			break;
		case BOOT_FINISHED:
			break;
		}
	}
}

void MAInjector::task_load(std::string task, int id, int address, int mapper_id, int mapper_address)
{
	std::cout << "Loading task ID " << id << " to PE " << (address >> 8) << "x" << (address & 0xFF) << std::endl;
	std::string path = "../management/"+task+"/repository.txt")
	std::ifstream repo(path);
	if(repo.is_open()){
		std::string line;
		std::getline(repo, line);
		int txt_size = std::stoi(line, nullptr, 16);
		std::getline(repo, line);
		int data_size = std::stoi(line, nullptr, 16);
		std::getline(repo, line);
		int bss_size = std::stoi(line, nullptr, 16);
		
		unsigned int packet_size = txt_size + CONSTANT_PACKET_SIZE;
		
		packet.clear();
		packet.reserve(packet_size);

		packet[0] = address;			/* Target address */
		packet[1] = packet_size - 2;	/* Packet size */
		packet[2] = TASK_ALLOCATION;	/* Platform service */
		packet[3] = id;					/* Task ID */
		packet[4] = mapper_address;		/* Mapper address */
		packet[8] = mapper_id;			/* Mapper ID */
		packet[9] = data_size;			/* Data section size */
		packet[10] = txt_size;			/* Text section size */
		packet[11] = bss_size;			/* BSS section size */


		int ptr_index  = CONSTANT_PACKET_SIZE; /* End of service header / payload start */

		/* Assembles payload */
		for(unsigned int i=0; i<code_size; i++){
			std::getline(repo, line);
			packet[ptr_index++] = std::stoi(line, nullptr, 16);
			// std::cout << line << std::endl;
		}
	} else {
		std::cout << "ERROR: cannot read the file at path " << path << std::endl;
	}
}

void MAInjector::ma_load()
{
	unsigned tasks_len = tasks.size() * 2;
	unsigned packet_size = CONSTANT_PACKET_SIZE + 2 + tasks_len;
	packet.clear();
	packet.reserve(packet_size);

	packet[0] = tasks[0].second;					/* Target: mapper address */
	packet[1] = packet_size - 2; 					/* Payload size */
	packet[2] = MESSAGE_DELIVERY;					/* Service */
	packet[3] = MA_INJECTOR_ADDRESS;				/* producer_task */
	packet[4] = 0x0000; 							/* consumer_task: mapper task id */
	packet[8] = tasks_len + 2; 						/* Message length */
	packet[CONSTANT_PACKET_SIZE] = MA_ALLOCATION;	/* Protocol: MA_ALLOCATION */
	packet[CONSTANT_PACKET_SIZE+1] = tasks_len;

	int ptr_index  = CONSTANT_PACKET_SIZE + 2; /* End of service header / payload start */

	for(int i = 0; i < tasks.size(); i++){
		packet[ptr_index++] = i;
		packet[ptr_index++] = tasks[i].second;
	}
}

void MAInjector::send_packet()
{
	static unsigned int aux_pkt[CONSTANT_PACKET_SIZE];
	static unsigned int aux_idx = 0;
	static int packet_idx = 0;
	static SEND_PKT_FSM last_state;

	if(reset.read()){
		send_pkt_state = SEND_IDLE;
	} else {
		switch(send_pkt_state){
		case SEND_IDLE:
			/* Trigger packet send by other FSMs */
			if(ma_boot_state == BOOT_WAIT || ma_boot_state == BOOT_SEND){
				if(credit_in.read()){
					if(!packet.empty()){
						send_pkt_state = SEND_PKT;
						packet_idx = 0;
					} else {
						cout << "ERROR: packet is empty at time " << tick_cnt <<  endl;
					}
				}
			} else if(ma_boot_state == BOOT_TASKS){
				/* Send a packet synchronously: DATA_AV -> MESSAGE_REQUEST -> MESSAGE_DELIVERY */
				if(credit_in.read()){
					/* Can send */
					aux_idx = 0;
					aux_pkt[0] = tasks[0].second;			/* Header: mapper task address */
					aux_pkt[1] = CONSTANT_PACKET_SIZE - 2;	/* Payload size */
					aux_pkt[2] = DATA_AV;					/* Service */
					aux_pkt[3] = MA_INJECTOR_ADDRESS;		/* producer_task */
					aux_pkt[4] = 0x0000; 					/* consumer_task: mapper task id */
					aux_pkt[8] = MA_INJECTOR_ADDRESS;		/* requesting_processor */

					send_pkt_state = SEND_DATA_AV;

					// std::cout << "AppInj: sending DATA_AV" << std::endl;
				}
			} else if(rcv_pkt_state == WAIT_SEND_REQ){
				if(credit_in.read()){
					/* Can send */
					aux_idx = 0;
					aux_pkt[0] = tasks[0].address; 			/* Header: mapper task address */
					aux_pkt[1] = CONSTANT_PACKET_SIZE - 2; 	/* Payload size */
					aux_pkt[2] = MESSAGE_REQUEST;			/* Service */
					aux_pkt[3] = 0x0000;					/* producer_task: mapper task id */
					aux_pkt[4] = MA_INJECTOR_ADDRESS; 		/* consumer_task */
					aux_pkt[8] = MA_INJECTOR_ADDRESS;		/* requesting_processor */

					send_pkt_state = SEND_MSG_REQ;

					// std::cout << "AppInj: sending MSG_REQUEST" << std::endl;
				}
			}
			break;
		case SEND_PKT:
			if(credit_in.read()){
				if(packet_idx < packet.size()){
					tx.write(1);
					data_out.write(packet[packet_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = SEND_FINISHED;
				}
			} else {
				tx.write(0);
				send_pkt_state = WAIT_CREDIT;
				last_state = SEND_PKT;
			}
			break;
		case SEND_FINISHED:
			if(!packet.empty())
				packet.clear();
			send_pkt_state = SEND_IDLE;
			// std::cout << "AppInj: Packet sent" << std::endl;
			break;
		case WAIT_CREDIT:
			if(credit_in.read()){
				tx.write(1);
				send_pkt_state = last_state;
			}
			break;
		case SEND_DATA_AV:
			if(credit_in.read()){
				if(aux_idx < CONSTANT_PACKET_SIZE){
					tx.write(1);
					data_out.write(aux_pkt[aux_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = WAIT_MSG_REQ;
					// std::cout << "AppInj: waiting MESSAGE_REQUEST" << std::endl;
				}
			} else {
				tx.write(0);
				send_pkt_state = WAIT_CREDIT;
				last_state = SEND_DATA_AV;
			}
			break;
		case WAIT_MSG_REQ:
			if(rcv_pkt_state == WAIT_SEND_DLVR){
				if(credit_in.read()){
					if(!packet.empty()){
						send_pkt_state = SEND_PKT;
						packet_idx = 0;
						// std::cout << "AppInj: Sending packet" << std::endl;
					} else {
						cout << "ERROR: packet is empty at time " << tick_cnt <<  endl;
					}
				}
			}
			break;
		case SEND_MSG_REQ:
			if(credit_in.read()){
				if(aux_idx < CONSTANT_PACKET_SIZE){
					tx.write(1);
					data_out.write(aux_pkt[aux_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = SEND_FINISHED;
				}
			} else {
				tx.write(0);
				send_pkt_state = WAIT_CREDIT;
				last_state = SEND_MSG_REQ;
			}
			break;

		}
	}
}

void MAInjector::rcv_packet(){
	static int payload_size;
	static int flit_counter;
	
	if(reset.read()){
		rcv_pkt_state = RCV_HEADER;
		sig_credit_out.write(1);
		rcv_packet.clear();
	} else if(clock.posedge()){
		
		/* Credit out update */
		if(
			send_pkt_state == WAIT_MSG_REQ || 
			ma_boot_state == BOOT_CONFIRM || (
				ma_boot_state == BOOT_MAP && 
				rcv_pkt_state != WAIT_SEND_REQ && 
				rcv_pkt_state != WAIT_ALLOCATION
			)
		){
			sig_credit_out.write(1);
		} else {
			sig_credit_out.write(0);
		}

		switch(rcv_pkt_state){
		case RCV_HEADER:
			if(rx.read() && sig_credit_out.read())
				rcv_pkt_state = RCV_PAYLOAD_SIZE;

			break;
		case RCV_PAYLOAD_SIZE:
			if(rx.read() && sig_credit_out.read()){
				payload_size = data_in.read();
				flit_counter = 0; // 1
				rcv_pkt_state = RCV_SERVICE;
			}
			break;
		case RCV_SERVICE:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
				switch(data_in.read()){
				case MESSAGE_REQUEST:
					rcv_pkt_state = RCV_MSG_REQ;
					break;
				case DATA_AV:
					rcv_pkt_state = RCV_DATA_AV;
					break;
				case MESSAGE_DELIVERY:
					rcv_pkt_state = RCV_MSG_DLVR;
					break;
				default:
					std::cout << "ERROR: unknown packet received at time " << tick_cnt << "\n" << std::endl;
					break;
				}
			}
			break;
		case RCV_MSG_REQ:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
				/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
				if(flit_counter == payload_size){
					rcv_pkt_state = WAIT_SEND_DLVR;
					// std::cout << "AppInj: MESSAGE_REQUEST received. Sending MESSAGE_DELIVERY" << std::endl;
				}
			}
			break;
		case WAIT_SEND_DLVR:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_DATA_AV:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
				/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
				if(flit_counter == payload_size){
					rcv_pkt_state = WAIT_SEND_REQ;
					// std::cout << "AppInj: received DATA_AV" << std::endl;
				}
			}
			break;
		case WAIT_SEND_REQ:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_MSG_DLVR:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
				/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
				unsigned int data = data_in.read();
				// cout << "F" << dec << flit_counter << ": " << hex << data << endl;
				if(flit_counter == 8){
					rcv_packet.clear();
					rcv_packet.reserve(data);
				} else if(flit_counter >= 13){
					rcv_packet[flit_counter - 13] = data;
				}

				if(flit_counter == payload_size){
					switch(rcv_packet[0]){
						case APP_ALLOCATION_REQUEST:
							// std::cout << "AppInj: received APP_ALLOCATION_REQUEST" << std::endl;
							// app_mapping_loader();
							rcv_pkt_state = WAIT_ALLOCATION;
							break;
						case APP_MAPPING_COMPLETE:
							// std::cout << "AppInj: received APP_MAPPING_COMPLETE" << std::endl;
							rcv_pkt_state = RCV_MAP_COMPLETE;
							break;
						default:
							cout << "ERROR: Unknown service " << hex << packet[0] << " received at time " << dec << current_time << endl;
							break;
					}
				}
			}
			break;
		case WAIT_ALLOCATION:
			if(ma_boot_state == BOOT_CONFIRM){
				rcv_pkt_state = RCV_HEADER;
			}
			break;
		case RCV_MAP_COMPLETE:
			rcv_pkt_state = RCV_HEADER;
			break;
		}
	}
}
