/*
 * AppInjector.cpp
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro on GAPH
 *
 *
 *  Description: This injector abstracts a external memory that sends new applications to the many-core system
 */

#include "AppInjector.hpp"

#include <array>

AppInjector::AppInjector(sc_module_name _name, std::string _path) : 
	sc_module(_name),
	path(_path)
{
	rcv_pkt_state = RCV_HEADER;
	send_pkt_state = SEND_IDLE;
	monitor_state = MONITOR_ACTIVE;

	std::ifstream ma_boot_info(path+"/ma_start.txt");
	std::string line;
	std::getline(ma_boot_info, line);
	if(line.compare("mapper_task") != 0){
		std::cout << "AppInjector: ERROR: first task must be 'mapper_task'" << std::endl;
		monitor_state = MONITOR_IDLE;
	} else {
		std::getline(ma_boot_info, line);
		mapper_address = std::stoi(line);
	}

	tick_cnt = 0;

	SC_METHOD(timer);
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

void AppInjector::timer()
{
	if(reset.read())
		tick_cnt = 0;
	else if(clock.posedge())
		tick_cnt++;
}

/**
 * @brief Monitors the file appstart.txt looking for a new app to inject on the system
 *
 * @details
 * MONITORING: Reads the next 4 lines of appstart.txt, extracting:
 * - app_name
 * - app_start_time
 * - app_task_number
 * - req_app_cluster_id (statically mapped cluster address)
 *
 * WAITING_TIME: Waits the simulation reach the time to fires a NEW_APP_REQ to the global manager
 * The appsstart.txt file had the applications sorted by its time to entry on the system.
 *
 * WAITING_SEND_APP_REQ: Waits the send_packet to send the NEW_APP_REQ to manager.
 * WAITING_ACK: Waits all the process to map a new application be completed by observing the state of
 * EA_receive_packet and EA_send_packet
 */
void AppInjector::monitor_new_app()
{
	static std::ifstream appstart(path+"/app_start.txt");
	static std::string name;
	static unsigned start_time;
	static unsigned task_cnt;
	static std::vector<int> static_mapping;
	static unsigned sent_task;

	if(reset.read() == 1){
		monitor_state = MONITOR_ACTIVE;
	} else if(clock.posedge()){
		switch(monitor_state){
		case MONITOR_IDLE:
			/* Wait mapping complete */
			if(rcv_pkt_state == RECEIVE_MAPPING_COMPLETE){
				monitor_state = MONITOR_ACTIVE;
				// std::cout << "AppInj: Mapping complete received. Entering monitoring." << std::endl;
			}
			break;
		case MONITOR_ACTIVE:
			if(appstart.is_open()){
				/* Reads the next line of the file. Supposed to be the application name */
				if(std::getline(appstart, name)){
					std::string line;

					/* Start time */
					std::getline(appstart, line);
					start_time = std::stoul(line);

					/* Task count */
					std::getline(appstart, line);
					task_cnt = std::stoul(line);

					// std::cout << "App name: " << name << std::endl;
					// std::cout << "Task count: " << task_cnt << std::endl;

					/* Gets static task mapping */
					for(unsigned i = 0; i < task_cnt; i++){
						std::getline(appstart, line);
						static_mapping.push_back(std::stoi(line));
						// std::cout << "Task id " << i << " mapped at " <<  static_mapping[i] << std::endl;
					}

					monitor_state = MONITOR_WAIT_TIME;
					// std::cout << "AppInj: app found. Will wait time" << std::endl;
				}
			} else {
				std::cout << "Unable to open file " << path << "/" << "app_start.txt" << std::endl;
			}
			break;
		case MONITOR_WAIT_TIME:
			if(rcv_pkt_state == RCV_HEADER && (start_time * 100000) <= tick_cnt){				
				// std::cout << "Time achieved! Will send new_app" << std::endl;
				app_descriptor_loader(name, task_cnt, static_mapping);
				monitor_state = MONITOR_SEND_NEW_APP;
			}
			break;
		case MONITOR_SEND_NEW_APP:
			if(send_pkt_state == SEND_FINISHED){
				sent_task = 0;
				monitor_state = MONITOR_MAP;
				// std::cout << "Descriptor sent (sync). Will enter SEND TASKS." << std::endl;
			}
			break;
		case MONITOR_MAP:
			if(rcv_pkt_state == RCV_WAIT_ALLOCATION){
				task_allocation_loader(packet_in[1 + sent_task*2], packet_in[1 + sent_task*2 + 1], 0, mapper_address);
				monitor_state = MONITOR_SEND_TASK;
			}
			break;
		case MONITOR_SEND_TASK:
			if(send_pkt_state == SEND_FINISHED){
				sent_task++;
				// std::cout << "Sent = " << sent_task << "; packet size = " << packet_in.size() << std::endl;
				if(1 + sent_task*2 < packet_in.size()){
					monitor_state = MONITOR_MAP;
				} else {
					monitor_state = MONITOR_IDLE;
				}
			}
			break;
		}
	}
}

/** 
 * @brief Reads application repository and fills the packet variable to sent it through the NoC
 */
void AppInjector::app_descriptor_loader(std::string name, unsigned task_cnt, std::vector<int>& static_mapping)
{
	std::string path = this->path+"/applications/"+name+".txt";
	std::ifstream repository(path);

	if(repository.is_open()){
		packet.clear();

		packet.push_back(mapper_address);		/* Header: mapper task address */
		packet.push_back(0);					/* Payload size: will be filled later */
		packet.push_back(MESSAGE_DELIVERY);		/* Service */
		packet.push_back(APP_INJECTOR_ADDRESS);	/* producer_task */
		packet.push_back(0x0000);				/* consumer_task: mapper task id */
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);					/* Message length: will be filled later */
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);

		packet.push_back(NEW_APP);				/* Protocol: NEW_APP */

		std::string line;
		std::getline(repository, line);			/* Task cnt */
		packet.push_back(task_cnt);

		for(unsigned i = 0; i < task_cnt; i++){
			packet.push_back(static_mapping[i]);			/* Task address from app_start.txt */

			packet.push_back(0x01);							/* Task type tag */

			std::getline(repository, line);		/* txt */
			std::getline(repository, line);		/* data */
			std::getline(repository, line);		/* bss */
			std::getline(repository, line);		/* addr */
		}

		static_mapping.clear();

		/* Base length is 2 flits per task + 2 header flits */
		unsigned message_len = task_cnt * 2 + 2;

		for(unsigned i = 0; i < task_cnt; i++){
			int successor = 0;
			do {
				std::getline(repository, line);
				successor = std::stol(line, nullptr, 16);	/* Consumers */
				packet.push_back(successor);
				message_len++;
			} while(successor > 0);
		}

		unsigned packet_size = CONSTANT_PACKET_SIZE + message_len;

		/* Fill previous flits */
		packet[1] = packet_size - 2;
		packet[8] = message_len;

		// for(unsigned i = 0; i < packet_size; i++){
		// 	std::cout << hex << packet[i] << std::endl;
		// }
		// std::cout << "APP_DESCRIPTOR SENT " << path << std::endl;

		repository.close();
	} else {
		std::cout << "Unable to open file " << path << std::endl;
	}
}

void AppInjector::credit_out_update()
{
	credit_out.write(sig_credit_out.read());
}

/**Converst an app ID to an app name by searching in app_start.txt file
 *
 */
std::string AppInjector::get_app_repo_path(unsigned app_id){
	std::string path = this->path+"/app_start.txt";
	ifstream repository(path);

	if(repository.is_open()){
		std::string line;
		for(unsigned app_cnt = 0; app_cnt < app_id; app_cnt++){
			/* Each descriptor in app_start start always with 3 information */
			for(int i = 0; i < 3; i++){
				std::getline(repository,line);
				// std::cout << line << std::endl;
			}
			unsigned task_cnt = std::stoul(line);
			// std::cout << "Task number: " << task_number << std::endl;

			/* Skips the proper number of tasks */
			for(unsigned i = 0; i < task_cnt; i++)
				std::getline(repository, line);
			
		}

		std::getline(repository,line);
		// std::cout << "APP NAME: " << line << std::endl;
		return (this->path + "/applications/" + line + ".txt");
	}

	std::cout << "ERROR: app path not found\n" << std::endl;
	return NULL;
}

/**Assembles the packet that load the a generic task to the system
 */
void AppInjector::task_allocation_loader(unsigned id, unsigned addr, unsigned mapper_id, unsigned mapper_addr){
	std::cout << "Loading task ID " << id << " to PE " << (addr >> 8) << "x" << (addr & 0xFF) << std::endl;

	unsigned app_id = id >> 8;
	unsigned task_id = id & 0xFF;

	std::string path = get_app_repo_path(app_id - 1);

	// std::cout << "Task allocation loader - app path: " << path << std::endl;

	ifstream repository(path);

	if(repository.is_open()){
		std::string line;

		std::getline(repository, line);
		unsigned task_cnt = std::stoul(line, nullptr, 16);

		if(task_id >= task_cnt)
			throw std::invalid_argument("ERROR[1] - task_id is out of range");

		/* Skips TASK_DESCRIPTOR_SIZE lines - TASK_DESCRIPTOR_SIZE is the size of each task description in repository.txt for each app */
		unsigned task_line = TASK_DESCRIPTOR_SIZE * task_id;
		for (unsigned i = 0; i < task_line; i++)
			std::getline(repository, line);

		std::getline(repository, line);
		unsigned txt_size = std::stoul(line, nullptr, 16);
		// std::cout << "Txt size: " << txt_size << std::endl;
		std::getline(repository, line);
		unsigned data_size = std::stoul(line, nullptr, 16);
		// std::cout << "Data size: " << data_size << std::endl;
		std::getline(repository, line);
		unsigned bss_size = std::stoul(line, nullptr, 16);
		// std::cout << "Bss size: " << bss_size << std::endl;

		std::getline(repository, line);
		unsigned init_addr = std::stoul(line, nullptr, 16);
		init_addr /= 4; /* Divided by 4 because memory has 4 byte words */

		// std::cout << "Current line: " << current_line << std::endl;

		/* Points the reader to the beggining of task code */
		for(unsigned current_line = task_line + TASK_DESCRIPTOR_SIZE + 1; current_line < init_addr; current_line++)
			std::getline(repository, line);
		
		// std::cout << "Task ID " << task_id << " code size " << txt_size << " code_line " << init_addr << std::endl;

		unsigned packet_size = (txt_size+data_size)/4 + CONSTANT_PACKET_SIZE;

		packet.clear();
		packet.reserve(packet_size);

		packet.push_back(addr);
		packet.push_back(packet_size - 2);
		packet.push_back(TASK_ALLOCATION);
		packet.push_back(id);
		packet.push_back(mapper_addr);
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(0);
		packet.push_back(mapper_id);
		packet.push_back(data_size);
		packet.push_back(txt_size);
		packet.push_back(bss_size);
		packet.push_back(0);

		for(unsigned i = 0; i < (txt_size+data_size)/4; i++){
			std::getline(repository, line);
			packet.push_back(std::stoul(line, nullptr, 16));
			// std::cout << line << std::endl;
		}
	} else {
		std::cout << "ERROR cannot read the file at path: " << path << " and app id " << app_id << std::endl;
	}
}

/** 
 * @brief This process controls the reading of packet incoming from NoC
 * 
 * @details
 * HEADER - Reads the packet header and goes to PAYLOAD_SIZE
 *
 * PAYLOAD_SIZE - Reads packet payload size and goes to SERVICE
 *
 * SERVICE - Verifies the service tipe. If service is APP_REQ_ACK, goes to RECEIVE_APP_ACK state.
 * If service is APP_ALLOCATION_REQUEST goes to RECEIVE_ALLOCATION_REQ state.

 * RECEIVE_APP_ACK - Extracts the manager address where the application was mapped, in sequence, loads the application description
 * from repository file by calling function "app_descriptor_loader". Such function creates a block of memory that store all relevant
 * information about the application and that need to be sent to the manager (which the application was mapped) through a NEW_APP packet.
 *
 * RECEIVE_ALLOCATION_REQ - Extracts the application task number and 4 relevant information about the application task, which are
 * embedded in the incoming packet. These information are (for each app task):
 *   ___________________________________________
 *  | id | repoaddr | code_size | allocatedproc |
 *
 *
 *  These information are stored in memory using the tasks_info pointer. After finish to receive the packe, this state
 *  calls the function "task_allocation_loader". Such function creates a continuous memory block (packet pointer), which
 *  stores all TASK_ALLOCATION packet, and that will be sent to the slave PE of each task.
 *
 *  OBS: The functions "task_allocation_loader" and "app_descriptor_loader" create a continuos block memory storing its address in
 *  pointer *packet and its size in the variable packet_size. Both pointer and variable are used by the send_packet function to send a
 *  packet to the NoC.
 */
void AppInjector::receive_packet()
{
	static unsigned payload_size;
	static unsigned flit_counter;

	if(reset.read()){
		rcv_pkt_state = RCV_HEADER;
		sig_credit_out.write(1);
		packet_in.clear();
	} else if(clock.posedge()){
		/* Credit out update */
		if(
			send_pkt_state == SEND_WAIT_REQUEST ||
			monitor_state == MONITOR_IDLE ||
			(
				monitor_state == MONITOR_MAP && 
				rcv_pkt_state != RCV_WAIT_REQ &&
				rcv_pkt_state != RCV_WAIT_ALLOCATION
			)
		)
			sig_credit_out.write(1);
		else
			sig_credit_out.write(0);
		
		switch(rcv_pkt_state){
		case RCV_HEADER:
			if(rx.read() && sig_credit_out.read())
				rcv_pkt_state = RCV_PAYLOAD_SIZE;

			break;
		case RCV_PAYLOAD_SIZE:
			if(rx.read() && sig_credit_out.read()){
				payload_size = data_in.read();
				flit_counter = 0;
				rcv_pkt_state = RCV_SERVICE;
			}
			break;
		case RCV_SERVICE:
			if(rx.read() && sig_credit_out.read()){
				switch(data_in.read()){
				case DATA_AV:
					rcv_pkt_state = RECEIVE_DATA_AV;
					break;
				case MESSAGE_REQUEST:
					rcv_pkt_state = RECEIVE_MESSAGE_REQUEST;
					break;
				case MESSAGE_DELIVERY:
					rcv_pkt_state = RECEIVE_MESSAGE_DELIVERY;
					break;
				default:
					std::cout << "ERROR: packet received unknown at time " << tick_cnt << "\n" << std::endl;
					break;
				}

				flit_counter++;
			}
			break;
		case RECEIVE_DATA_AV:
			if(rx.read() && sig_credit_out.read()){
				flit_counter++;
			}

			/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
			if(flit_counter == payload_size){
				rcv_pkt_state = RCV_WAIT_REQ;
				// std::cout << "AppInj: received DATA_AV" << std::endl;
			}
			break;
		case RECEIVE_MESSAGE_REQUEST:
			if(rx.read() && sig_credit_out.read()){
				// std::cout << "AppInj: flit = " << flit_counter << "; data = " << data_in.read() << std::endl;
				flit_counter++;
			}

			/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
			if(flit_counter == payload_size){
				rcv_pkt_state = RCV_WAIT_DLVR;
				// std::cout << "AppInj: MESSAGE_REQUEST received. Sending MESSAGE_DELIVERY" << std::endl;
				// std::cout << "AppInj: delivery packet size is " << packet.size() << std::endl;
			}
			break;
		case RECEIVE_MESSAGE_DELIVERY:
			if(rx.read() && sig_credit_out.read()){
				/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
				unsigned int data = data_in.read();
				// std::cout << "AppInj: flit = " << flit_counter << "; data = " << data << std::endl;
				// cout << "F" << dec << flit_counter << ": " << hex << data << endl;
				if(flit_counter == 8 - 2){
					packet_in.clear();
					packet_in.reserve(data);
					// std::cout << "AppInj: Delivery size = " << data << std::endl;
				} else if(flit_counter >= CONSTANT_PACKET_SIZE - 2){
					// std::cout << "AppInj: index = " << flit_counter - (CONSTANT_PACKET_SIZE - 2) << "; data = " << data << std::endl;
					packet_in.push_back(data);
				}

				flit_counter++;
			}

			if(flit_counter == payload_size){
				switch(packet_in[0]){
				case APP_ALLOCATION_REQUEST:
					// std::cout << "AppInj: received APP_ALLOCATION_REQUEST" << std::endl;
					rcv_pkt_state = RCV_WAIT_ALLOCATION;
					break;
				case APP_MAPPING_COMPLETE:
					// std::cout << "AppInj: received APP_MAPPING_COMPLETE" << std::endl;
					rcv_pkt_state = RECEIVE_MAPPING_COMPLETE;
					break;
				default:
					std::cout << "ERROR: Unknown service " << hex << packet[0] << " received at time " << dec << tick_cnt << std::endl;
					break;
				}
			}
			break;
		case RCV_WAIT_REQ:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_WAIT_DLVR:
			if(send_pkt_state == SEND_FINISHED)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RCV_WAIT_ALLOCATION:
			if(monitor_state == MONITOR_IDLE)
				rcv_pkt_state = RCV_HEADER;

			break;
		case RECEIVE_MAPPING_COMPLETE:
			rcv_pkt_state = RCV_HEADER;
			packet_in.clear();
			break;
		}
	}
}

/**
 * @brief Only is in charge to send data to NoC using as reference the packet pointer and packet_size variable.
 */
void AppInjector::send_packet(){
	static SEND_PKT_FSM last_state;
	static std::array<unsigned, CONSTANT_PACKET_SIZE> aux_pkt;
	static unsigned int aux_idx = 0;
	static unsigned int pkt_idx = 0;

	if(reset.read() == 1) {
		send_pkt_state = SEND_IDLE;
	} else {
		switch(send_pkt_state){
		case SEND_IDLE:
			if(monitor_state == MONITOR_SEND_TASK){
				if (credit_in.read() == 1){
					if(!packet.empty()) {
						send_pkt_state = SEND_PACKET;
						pkt_idx = 0;
					} else {
						std::cout << "ERROR: packet has an NULL pointer at time " << tick_cnt <<  std::endl;
					}
				}
			} else if(monitor_state == MONITOR_SEND_NEW_APP){
				/* Send a packet synchronously: DATA_AV -> MESSAGE_REQUEST -> MESSAGE_DELIVERY */
				if(credit_in.read()){
					/* Can send */
					aux_idx = 0;
					aux_pkt[0] = mapper_address;			/* Header: mapper task address */
					aux_pkt[1] = CONSTANT_PACKET_SIZE - 2;	/* Payload size */
					aux_pkt[2] = DATA_AV;					/* Service */
					aux_pkt[3] = APP_INJECTOR_ADDRESS;		/* producer_task */
					aux_pkt[4] = 0x0000; 					/* consumer_task: mapper task id */
					aux_pkt[8] = APP_INJECTOR_ADDRESS;		/* requesting_processor */

					send_pkt_state = SEND_DATA_AV;

					// std::cout << "AppInj: sending DATA_AV" << std::endl;
				}
			} else if(rcv_pkt_state == RCV_WAIT_REQ){
				if(credit_in.read()){
					/* Can send */
					aux_idx = 0;
					aux_pkt[0] = mapper_address; 			/* Header: mapper task address */
					aux_pkt[1] = CONSTANT_PACKET_SIZE - 2; 	/* Payload size */
					aux_pkt[2] = MESSAGE_REQUEST;			/* Service */
					aux_pkt[3] = 0x0000;					/* producer_task: mapper task id */
					aux_pkt[4] = APP_INJECTOR_ADDRESS; 		/* consumer_task */
					aux_pkt[8] = APP_INJECTOR_ADDRESS;		/* requesting_processor */

					send_pkt_state = SEND_MSG_REQUEST;

					// std::cout << "AppInj: sending MSG_REQUEST" << std::endl;
				}
			}
			break;
		case SEND_DATA_AV:
			if(credit_in.read() == 1){
				if(aux_idx < CONSTANT_PACKET_SIZE){
					tx.write(1);
					data_out.write(aux_pkt[aux_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = SEND_WAIT_REQUEST;
					// std::cout << "AppInj: waiting MESSAGE_REQUEST" << std::endl;
				}
			} else {
				tx.write(0);
				send_pkt_state = SEND_WAIT_CREDIT;
				last_state = SEND_DATA_AV;
			}
			break;
		case SEND_MSG_REQUEST:
			if(credit_in.read() == 1){
				if(aux_idx < CONSTANT_PACKET_SIZE){
					tx.write(1);
					data_out.write(aux_pkt[aux_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = SEND_FINISHED;
				}
			} else {
				tx.write(0);
				send_pkt_state = SEND_WAIT_CREDIT;
				last_state = SEND_MSG_REQUEST;
			}
			break;
		case SEND_WAIT_REQUEST:
			if(rcv_pkt_state == RCV_WAIT_DLVR){
				if(credit_in.read() == 1){
					if(!packet.empty()){
						send_pkt_state = SEND_PACKET;
						pkt_idx = 0;
						// std::cout << "AppInj: Sending packet" << std::endl;
					} else {
						cout << "ERROR: packet is empty at time " << tick_cnt <<  endl;
					}
				}
			}
			break;
		case SEND_PACKET:
			if(credit_in.read()){
				if(pkt_idx < packet.size()){
					// std::cout << "Write: " << packet[packet_idx] << std::endl;
					tx.write(1);
					data_out.write(packet[pkt_idx++]);
				} else {
					tx.write(0);
					send_pkt_state = SEND_FINISHED;
				}
			} else {
				tx.write(0);
				send_pkt_state = SEND_WAIT_CREDIT;
				last_state = SEND_PACKET;
			}
			break;
		case SEND_WAIT_CREDIT:
			if(credit_in.read() == 1){
				tx.write(1);
				send_pkt_state = last_state;
			}
			break;
		case SEND_FINISHED:
			send_pkt_state = SEND_IDLE;
			// std::cout << "AppInj: Packet sent" << std::endl;
			break;
		}
	}
}
