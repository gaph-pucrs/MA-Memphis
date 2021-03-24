/*
 * app_injector.cpp
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro on GAPH
 *
 *
 *  Description: This injector abstracts a external memory that sends new applications to the many-core system
 */

#include "MAInjector.hpp"

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
	static std::ifstream ma_boot_info("maboot.txt");
	static int ma_boot_task_cnt = 0;
	static unsigned sent_task = 0;

	if(reset.read()){
		ma_boot_state = BOOT_INIT;
	} else if(clock.posedge()){
		switch(ma_boot_state){
		case BOOT_INIT:
			if(ma_boot_info.is_open()){
				tasks.clear();
				std::string task_name;
				ma_boot_task_cnt = 0;
				while(std::getline(ma_boot_info, task_name)){
					std::string addrstr;
					std::getline(ma_boot_info, addrstr);
					// std::cout << "maboot.txt line: " << addrstr << std::endl;
					int address = std::stoi(addrstr);
					// std::cout << "Converted value: " << address << std::endl;

					tasks.push_back(std::make_pair(task_name, address));

					// std::cout << "Task name: " << task_name << std::endl;
					// std::cout << "Address: " << address << std::endl;
					ma_boot_task_cnt++;
				}

				// std::cout << "MA boot task count: " << ma_boot_task_cnt << std::endl;

				/* First task should be the mapper directly injected to the system with static mapping */
				if(tasks[0].first.compare("mapper_task") != 0){
					std::cout << "ERROR: first task must be 'mapper_task'" << std::endl;
					ma_boot_state = BOOT_FINISHED;
				}

				if(tasks[0].second == -1){
					std::cout << "ERROR: 'mapper_task' should be static mapped" << std::endl;
					ma_boot_state = BOOT_FINISHED;
				}

				task_load(tasks[0].first, 0, tasks[0].second, -1, -1);

				ma_boot_state = BOOT_WAIT;
			} else {
				std::cout << "ERROR: Unable to open file mastart.txt" << std::endl;
			}
			break;
		case BOOT_WAIT:
			/* Waits ends of boot packet transmission */
			if(send_pkt_state == SEND_FINISHED){
				// std::cout << "bootloader send ok" << std::endl;

				/* Now will request the MA mapping */
				/* All MA startup information is already loaded into 'tasks' structure */
				ma_load();

				ma_boot_state = BOOT_TASKS;
			}
			break;
		case BOOT_TASKS:
			if(send_pkt_state == SEND_FINISHED){
				sent_task = 1;	/* Start at 1. Mapper task is already mapped */
				ma_boot_state = BOOT_MAP;
			}
			break;
		case BOOT_MAP:
			if(rcv_pkt_state == WAIT_ALLOCATION){
				task_load(tasks[packet_in[1 + sent_task*2]].first, packet_in[1 + sent_task*2], packet_in[1 + sent_task*2 + 1], 0, tasks[0].second);
				ma_boot_state = BOOT_SEND;
			}
			break;
		case BOOT_SEND:
			if(send_pkt_state == SEND_FINISHED){
				sent_task++;
				// std::cout << "Sent = " << sent_task << "; packet size = " << packet_in.size() << std::endl;
				if(1 + sent_task*2 < packet_in.size()){
					ma_boot_state = BOOT_MAP;
				} else {

					ma_boot_state = BOOT_CONFIRM;
				}
			}
			break;
		case BOOT_CONFIRM:
			if(rcv_pkt_state == RCV_MAP_COMPLETE){
				packet.clear();
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
	std::string path = "../management/"+task+"/repository.txt";
	std::ifstream repo(path);

	if(repo.is_open()){
		std::string line;
		std::getline(repo, line);	/* Task type tag */

		std::getline(repo, line);
		unsigned txt_size = std::stoul(line, nullptr, 16);
		// std::cout << "Txt size: " << txt_size << std::endl;

		std::getline(repo, line);
		unsigned data_size = std::stoul(line, nullptr, 16);
		// std::cout << "Data size: " << data_size << std::endl;

		std::getline(repo, line);
		unsigned bss_size = std::stoul(line, nullptr, 16);
		// std::cout << "Bss size: " << bss_size << std::endl;
		
		unsigned packet_size = txt_size + CONSTANT_PACKET_SIZE;
		
		packet.clear();
		packet.reserve(packet_size);

		packet.push_back(address);			/* Target address */
		packet.push_back(packet_size - 2);	/* Packet size */
		packet.push_back(TASK_ALLOCATION);	/* Platform service */
		packet.push_back(id);					/* Task ID */
		packet.push_back(mapper_address);		/* Mapper address */
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(mapper_id);				/* Mapper ID */
		packet.push_back(data_size);				/* Data section size */
		packet.push_back(txt_size);				/* Text section size */
		packet.push_back(bss_size);				/* BSS section size */
		packet.push_back(0);

		// std::cout << "PACKET SIZE: " << packet.size() << std::endl;

		/* Assembles payload */
		for(unsigned i = 0; i < txt_size; i++){
			std::getline(repo, line);
			// std::cout << line << std::endl;
			packet.push_back(std::stoul(line, nullptr, 16));
			// std::cout << packet[ptr_index - 1] << std::endl;
		}
		// std::cout << "Generated packet size: " << packet.size() << std::endl;
	} else {
		std::cout << "ERROR: cannot read the file at path " << path << std::endl;
	}
}

void MAInjector::ma_load()
{
	/* Memphis packet + message protocol (type + size) + task address */
	unsigned packet_size = CONSTANT_PACKET_SIZE + 2 + tasks.size()*TASK_DESCRIPTOR_SIZE + 1;

	packet.clear();
	packet.reserve(packet_size);

	packet.push_back(tasks[0].second);		/* Target: mapper address */
	packet.push_back(packet_size - 2); 		/* Payload size */
	packet.push_back(MESSAGE_DELIVERY);		/* Service */
	packet.push_back(MA_INJECTOR_ADDRESS);	/* producer_task */
	packet.push_back(0x0000); 				/* consumer_task: mapper task id */
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(tasks.size()*TASK_DESCRIPTOR_SIZE + 1 + 2);	/* Message length */
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(0);

	packet.push_back(NEW_APP);	/* Protocol: MA_ALLOCATION */
	packet.push_back(tasks.size()*TASK_DESCRIPTOR_SIZE + 1);		/* Descriptor size */

	packet.push_back(tasks.size());

	for(unsigned i = 0; i < tasks.size(); i++){
		std::string path = "../management/" + tasks[i].first + "/repository.txt";
		std::ifstream repo(path);

		packet.push_back(i);				/* Task ID */
		packet.push_back(tasks[i].second);	/* Task Address */

		std::string line;

		std::getline(repo, line);
		// std::cout << "TTT: " << line << std::endl;
		unsigned task_type_tag = std::stoul(line, nullptr, 16);
		packet.push_back(task_type_tag);

		std::getline(repo, line);
		// std::cout << "txt_size: " << line << std::endl;
		unsigned txt_size = std::stoul(line, nullptr, 16);
		packet.push_back(txt_size);

		std::getline(repo, line);
		// std::cout << "data_size: " << line << std::endl;
		unsigned data_size = std::stoul(line, nullptr, 16);
		packet.push_back(data_size);

		std::getline(repo, line);
		// std::cout << "bss_size: " << line << std::endl;
		unsigned bss_size = std::stoul(line, nullptr, 16);
		packet.push_back(bss_size);

		packet.push_back(0);			/* Initial address */
	}

	// std::cout << "MAInjector: NEW_APP packet size = " << packet.size() << std::endl;
}

void MAInjector::send_packet()
{
	static std::array<unsigned, CONSTANT_PACKET_SIZE> aux_pkt;
	static unsigned int aux_idx = 0;
	static unsigned packet_idx = 0;
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
						// std::cout << "MAInjector: Packet send triggered" << std::endl;
						send_pkt_state = SEND_PKT;
						packet_idx = 0;
					} else {
						std::cout << "ERROR: packet is empty at time " << tick_cnt << std::endl;
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

					// std::cout << "MAInjector: sending DATA_AV" << std::endl;
				}
			} else if(rcv_pkt_state == WAIT_SEND_REQ){
				if(credit_in.read()){
					/* Can send */
					aux_idx = 0;
					aux_pkt[0] = tasks[0].second; 			/* Header: mapper task address */
					aux_pkt[1] = CONSTANT_PACKET_SIZE - 2; 	/* Payload size */
					aux_pkt[2] = MESSAGE_REQUEST;			/* Service */
					aux_pkt[3] = 0x0000;					/* producer_task: mapper task id */
					aux_pkt[4] = MA_INJECTOR_ADDRESS; 		/* consumer_task */
					aux_pkt[8] = MA_INJECTOR_ADDRESS;		/* requesting_processor */

					send_pkt_state = SEND_MSG_REQ;

					// std::cout << "MAInjector: sending MSG_REQUEST" << std::endl;
				}
			}
			break;
		case SEND_PKT:
			if(credit_in.read()){
				if(packet_idx < packet.size()){
					// std::cout << "Write: " << packet[packet_idx] << std::endl;
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
			send_pkt_state = SEND_IDLE;
			// std::cout << "MAInjector: Packet sent" << std::endl;
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
					// std::cout << "MAInjector: waiting MESSAGE_REQUEST" << std::endl;
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
						// std::cout << "MAInjector: Sending packet" << std::endl;
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
	static unsigned payload_size;
	static unsigned flit_counter;
	
	if(reset.read()){
		rcv_pkt_state = RCV_HEADER;
		sig_credit_out.write(1);
		packet_in.clear();
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
			if(rx.read() && sig_credit_out.read()){
				rcv_pkt_state = RCV_PAYLOAD_SIZE;
				// std::cout << "MAInjector: received header" << std::endl;
			}

			break;
		case RCV_PAYLOAD_SIZE:
			if(rx.read() && sig_credit_out.read()){
				payload_size = data_in.read();
				flit_counter = 0;
				rcv_pkt_state = RCV_SERVICE;
				// std::cout << "MAInjector: received payload size = " << payload_size << std::endl;
			}
			break;
		case RCV_SERVICE:
			if(rx.read() && sig_credit_out.read()){
				switch(data_in.read()){
				case MESSAGE_REQUEST:
					rcv_pkt_state = RCV_MSG_REQ;
					// std::cout << "MAInjector: received MR packet" << std::endl;
					break;
				case DATA_AV:
					rcv_pkt_state = RCV_DATA_AV;
					// std::cout << "MAInjector: received DA packet" << std::endl;
					break;
				case MESSAGE_DELIVERY:
					rcv_pkt_state = RCV_MSG_DLVR;
					// std::cout << "MAInjector: received MD packet" << std::endl;
					break;
				default:
					std::cout << "ERROR: unknown packet received at time " << tick_cnt << "\n" << std::endl;
					break;
				}

				flit_counter++;
			}
			break;
		case RCV_MSG_REQ:
			if(rx.read() && sig_credit_out.read()){
				// std::cout << "MAInjector: flit = " << flit_counter << "; data = " << data_in.read() << std::endl;
				flit_counter++;
			}

			/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
			if(flit_counter == payload_size){
				rcv_pkt_state = WAIT_SEND_DLVR;
				// std::cout << "MAInjector: MESSAGE_REQUEST received. Sending MESSAGE_DELIVERY" << std::endl;
				// std::cout << "MAInjector: delivery packet size is " << packet.size() << std::endl;
			}
			break;
		case WAIT_SEND_DLVR:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_DATA_AV:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
			}

			/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
			if(flit_counter == payload_size){
				rcv_pkt_state = WAIT_SEND_REQ;
				// std::cout << "MAInjector: received DATA_AV" << std::endl;
			}
			break;
		case WAIT_SEND_REQ:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_MSG_DLVR:
			if(rx.read() && sig_credit_out.read()){
				/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
				unsigned int data = data_in.read();
				// std::cout << "MAInjector: flit = " << flit_counter << "; data = " << data << std::endl;
				// cout << "F" << dec << flit_counter << ": " << hex << data << endl;
				if(flit_counter == 8 - 2){
					packet_in.clear();
					packet_in.reserve(data);
					// std::cout << "MAInjector: Delivery size = " << data << std::endl;
				} else if(flit_counter >= CONSTANT_PACKET_SIZE - 2){
					// std::cout << "MAInjector: index = " << flit_counter - (CONSTANT_PACKET_SIZE - 2) << "; data = " << data << std::endl;
					packet_in.push_back(data);
				}

				flit_counter++;
			}

			if(flit_counter == payload_size){
				switch(packet_in[0]){
				case APP_ALLOCATION_REQUEST:
					// std::cout << "MAInjector: received APP_ALLOCATION_REQUEST" << std::endl;
					// app_mapping_loader();
					rcv_pkt_state = WAIT_ALLOCATION;
					break;
				case APP_MAPPING_COMPLETE:
					// std::cout << "MAInjector: received APP_MAPPING_COMPLETE" << std::endl;
					rcv_pkt_state = RCV_MAP_COMPLETE;
					break;
				default:
					std::cout << "ERROR: Unknown service " << hex << packet[0] << " received at time " << dec << tick_cnt << std::endl;
					break;
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
			packet_in.clear();
			break;
		}
	}
}
