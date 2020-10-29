/*
 * app_injector.cpp
 *
 *  Created on: 6 de ago de 2018
 *      Author: Marcelo Ruaro on GAPH
 *
 *
 *  Description: This injector abstracts a external memory that sends new applications to the many-core system
 */

#include "app_injector.h"

//This line enables the integration with vhdl
#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(app_injector);
#endif

void app_injector::credit_out_update(){
	credit_out.write(sig_credit_out.read());
}

/**Converst an app ID to an app name by searching in app_start.txt file
 *
 */
string app_injector::get_app_repo_path(unsigned int app_id){
	string line;
	string path = "appstart.txt";
	ifstream repo_file (path.c_str());
	string app_name;
	unsigned int task_number;

	if (repo_file.is_open()) {

		for(unsigned int app_count = 0; app_count < app_id; app_count++){

			/*Each descriptor in app_start start always with 4 information*/
			for(int i=0; i<4; i++){
				getline (repo_file,line);
			}
			sscanf( line.substr(0, 8).c_str(), "%u", &task_number);

			/*Skips the number of tasks*/
			for(unsigned int i=0; i<task_number; i++){
				getline (repo_file,line);
			}

		}

		getline (repo_file,line);
		return ("../applications/" + line + "/repository.txt");
	}

	cout << "ERROR: app path not found\n" << endl;
	return NULL;

}

/**Assembles the packet that load the a generic task to the system
 */
void app_injector::task_allocation_loader(unsigned int id, unsigned int addr, unsigned int mapper_id, unsigned int mapper_addr){

	string line, path;
	unsigned int  task_number, code_size, data_size, bss_size, task_line, code_line, current_line;
	int ptr_index = 0;
	unsigned int app_id, task_id;

	app_id = id >> 8;
	task_id = id & 0xFF;

	cout << "Loading task ID " << id << " to PE " << (addr >> 8) << "x" << (addr & 0xFF) << endl;


	path = get_app_repo_path(app_id);

	// cout << "Task allocation loader - app path: " << path << endl;

	ifstream repo_file (path.c_str());

	if (repo_file.is_open()) {

		getline (repo_file,line);
		sscanf( line.substr(0, 8).c_str(), "%x", &task_number);

		if (task_id+1 > task_number)
			throw std::invalid_argument("ERROR[1] - task_id is out of range");

		task_line = (TASK_DESCRIPTOR_SIZE * task_id);
		/*Skips TASK_DESCRIPTOR_SIZE lines - TASK_DESCRIPTOR_SIZE is the size of each task description in repository.txt for each app*/
		for (unsigned int i=0; i < task_line; i++)
			getline (repo_file,line);

		getline (repo_file,line); /*Task ID*/
		getline (repo_file,line); /*static mapped PE*/
		getline (repo_file,line); /*code size*/
		sscanf( line.substr(0, 8).c_str(), "%x", &code_size);
		getline (repo_file,line); /*data size*/
		sscanf( line.substr(0, 8).c_str(), "%x", &data_size);
		getline (repo_file,line); /*bss size*/
		sscanf( line.substr(0, 8).c_str(), "%x", &bss_size);
		getline (repo_file,line); /*initial_address*/
		sscanf( line.substr(0, 8).c_str(), "%x", &code_line);

		code_line = code_line / 4; /*Divided by 4 because memory has 4 byte words*/

		current_line = task_line + TASK_DESCRIPTOR_SIZE + 1; /*Finds the current line by sum the number of task by the task decription size*/

		//cout << "Current line: " << current_line << endl;
		/*Points the reader to the beging of task code*/
		while(current_line < code_line){
			getline (repo_file,line);
			current_line++;
		}
		//cout << "Task ID " << task_id << " code size " << code_size << " code_line " << code_line << endl;

		packet_size = code_size+CONSTANT_PACKET_SIZE;

		packet = new unsigned int[packet_size];

		packet[0] = addr; //Packet service
		packet[1] = packet_size-2; //Packet service
		packet[2] = TASK_ALLOCATION; //Packet service
		packet[3] = id;
		packet[4] = mapper_addr;
		packet[8] = mapper_id;
		packet[9] = data_size; //Data size
		packet[10] = code_size; //Code size
		packet[11] = bss_size; //Bss size
		ptr_index  = CONSTANT_PACKET_SIZE; //Jumps to the end of ServiceHeader

		//Assembles txt
		for(unsigned int i=0; i<code_size; i++){
			getline (repo_file,line);
			sscanf( line.substr(0, 8).c_str(), "%x", &packet[ptr_index++]);
			//cout << line << endl;
		}

	} else {
		cout << "ERROR cannot read the file at path: " << path << " and app id " << app_id << endl;
	}
}

void app_injector::bootloader(){
	if (reset.read() == 1)  {
		EA_bootloader = INITIALIZE;

	} else if (clock.posedge()){

		switch (EA_bootloader) {

			/*Sends the Global Mapper App to PE 0 */
			case INITIALIZE:
				/*Load the boot task in the packet array*/
				// cout << "bootloader init task alloc loader " << endl;
				task_allocation_loader(0, 0, -1, -1);
				/*This state signals to send_packet to start transmission*/
				EA_bootloader = WAIT_SEND_BOOT;
				// cout << "bootloader init ok" << endl;
				break;
			case WAIT_SEND_BOOT:
				/*Waits ends of boot packet transmission*/
				if (EA_send_packet == SEND_FINISHED){
					EA_bootloader = BOOTLOADER_FINISHED;
					// cout << "bootloader send ok" << endl;
				}
				break;
			case BOOTLOADER_FINISHED:
				break;
		}
	}
}


/**
 * Monitors the file appstart.txt looking for a new app to inject on the system
 *
 * MONITORING: Reads the next 4 lines of appsstart.txt, extracting:
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
void app_injector::monitor_new_app(){
	string line;
	ifstream appstart_file;

	if (reset.read() == 1)  {
		EA_new_app_monitor = MONITORING;
		current_time = 0;
		req_app_start_time = 0;
		req_app_task_number = 0;
		req_app_cluster_id = 0;
		line_counter = MAN_APP_DESCRIPTOR_SIZE;

	} else if (clock.posedge()){

		switch (EA_new_app_monitor) {

			case IDLE_MONITOR:
				//Waits until the master sends the app mapping complete
				if (EA_receive_packet == RECEIVE_MAPPING_COMPLETE)
					EA_new_app_monitor = MONITORING;

				break;

			case MONITORING: //Reads appstart.txt

				appstart_file.open("appstart.txt", std::ifstream::in);

				if (appstart_file.is_open()) {

					//Points to the last line read
					for (unsigned int i=0; i < line_counter; i++)
						getline (appstart_file,line);

					//Reads the next line of the file. Supposed to be the application name
					getline (appstart_file,req_app_name);
					// cout << "found app name " << req_app_name << endl;
					if (req_app_name != "deadc0de"){

						//Gets the application start time
						getline (appstart_file,line);
						sscanf( line.substr(0, 8).c_str(), "%u", &req_app_start_time ); //Start time is in milliseconds

						// //Gets the application cluster
						// getline (appstart_file,line);
						// sscanf( line.substr(0, 8).c_str(), "%d", &req_app_cluster_id );

						//Gets the application task number
						getline (appstart_file,line);
						sscanf( line.substr(0, 8).c_str(), "%u", &req_app_task_number );

						line_counter = line_counter + 3;

						task_static_mapping = new int[req_app_task_number];

						// cout << "App name: " << req_app_name << endl;
						// cout << "req_app_cluster_id: " << req_app_cluster_id << endl;
						// cout << "app_task_number: " << req_app_task_number << endl;		/** @todo ALWAYS -1, locks whole process */

						//Gets the allocated processor for each task, useful for static task mapping
						for(unsigned int i=0; i<req_app_task_number; i++){
							//Gets the application task number
							getline (appstart_file,line);
							sscanf( line.substr(0, 8).c_str(), "%d", &task_static_mapping[i]);
							// cout << "task id " << i << " mapped at " <<  task_static_mapping[i] << endl;
							line_counter++;
						}

						EA_new_app_monitor = WAITING_TIME;
						// cout << "app found. will wait time" << endl;
					}

					appstart_file.close();

				} else {
					cout << "Unable to open file appstart.txt" << endl;
				}

				break;

			case WAITING_TIME: //Test when the current time reach the application start time

				if (EA_receive_packet == HEADER && EA_bootloader == BOOTLOADER_FINISHED && (req_app_start_time * 100000) <= current_time){
					/* It once sent "NEW_APP_REQ" */
					/* Now it sends direclty NEW_APP -> no clustering */
						
					//Loads app descriptor to the pointer * packet (used in send_packet function)
					// cout << "time achieved! will send new_app" << endl;
					app_descriptor_loader();
					EA_new_app_monitor = WAITING_SEND_NEW_APP;
				}

				break;

			case WAITING_SEND_NEW_APP:
				if (EA_send_packet == SEND_FINISHED)
					EA_new_app_monitor = IDLE_MONITOR;

				break;
		}

		current_time++;
	}
}

/* Reads application repository and fills the packet variable to sent it through the NoC
 * This function MODIFIES the following global variables:
 * - *packet = receives a pointer to the allocated memory block
 * - packet_size = receives the packet size
 *
 * This function USES the following global variables:
 * - app_name = used to form the application repository's path
 * - app_task_number = used to compute the repository's size
 */
void app_injector::app_descriptor_loader(){

	string line;
	string path = "../applications/" + req_app_name + "/repository.txt";
	ifstream repo_file (path.c_str());
	int file_length;
	int ptr_index;
	int allocated_proc_index;
	int task_index;

	file_length = 0;
	ptr_index = 0;

	if (repo_file.is_open()) {

		file_length = (TASK_DESCRIPTOR_SIZE * req_app_task_number) + 1; //Plus one because the descriptors stores the information of task number at the firt line

		// Points to the begging of file
		repo_file.clear();
		repo_file.seekg (0, repo_file.beg);

		//Sets the NoC's packet size
		packet_size = CONSTANT_PACKET_SIZE + 2 + file_length;

		//Allocate memory
		packet = new unsigned int[packet_size];

		//Assembles the Service Header on packet
		packet[0] = 0x0000; 		 			/* Header: mapper task address */
		packet[1] = packet_size - 2; 			/* Payload size */
		packet[2] = MESSAGE_DELIVERY;			/* Service */
		packet[3] = APP_INJECTOR_ADDRESS;		/* producer_task */
		packet[4] = 0x0000; 					/* consumer_task: mapper task id */
		packet[8] = file_length + 2; 			/* Message length */
		packet[CONSTANT_PACKET_SIZE] = NEW_APP;	/* Protocol: NEW_APP */
		packet[CONSTANT_PACKET_SIZE+1] = file_length;

		ptr_index = CONSTANT_PACKET_SIZE + 2; //ptr_index starts after ServiceHeader + 2

		//Assembles the App Descriptor from repository file
		allocated_proc_index = 2;//Starts at index 2 because the first index is used to the number of app tasks, and the second one to the task name
		task_index = 0;

		for(int i=0; i<file_length; i++){

			getline (repo_file,line);

			if(i == allocated_proc_index){//If the current line is the allocated proc, then inserts the statically mapped process address
				packet[ptr_index++] = task_static_mapping[task_index++];
				allocated_proc_index += TASK_DESCRIPTOR_SIZE; //Jumps the index to the next field of allocated proc
			} else
				sscanf( line.substr(0, 8).c_str(), "%x", &packet[ptr_index++]);
		}

		delete [] task_static_mapping;
		task_static_mapping = NULL;

		// for(unsigned int i=0; i < packet_size; i++){
		// 	cout << hex << packet[i] << endl;
		// }
		// cout << "APP_DESCRIPTOR SENT " << path << endl;

		repo_file.close();
	} else {
		cout << "Unable to open file " << path << endl;
	}

}


/** This process controls the reading of packet incoming from NoC
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
void app_injector::receive_packet(){

	if(reset.read()){
		EA_receive_packet = HEADER;
		req_task_id = 0;
		req_task_allocated_proc = 0;
		req_task_master_ID = 0;
		req_task_id_real = 0;
		sig_credit_out.write(1);
	} else {
		/* Credit out update */
		if(
			EA_send_packet == WAIT_MESSAGE_REQUEST ||
			(
				EA_new_app_monitor == IDLE_MONITOR && 
				EA_receive_packet != WAIT_SEND_REQUEST &&
				EA_receive_packet != WAITING_SEND_TASK_ALLOCATION
			)
		)
			sig_credit_out.write(1);
		else
			sig_credit_out.write(0);
		
		switch(EA_receive_packet){
			case HEADER:
				if(rx.read() && sig_credit_out.read())
					EA_receive_packet = PAYLOAD_SIZE;

				break;
			case PAYLOAD_SIZE:
				if(rx.read() && sig_credit_out.read()){
					payload_size = data_in.read();
					flit_counter = 1;
					EA_receive_packet = SERVICE;
				}
				break;
			case SERVICE:
				if(rx.read() && sig_credit_out.read()){
					switch(data_in.read()){
						case DATA_AV:
							EA_receive_packet = RECEIVE_DATA_AV;
							break;
						case MESSAGE_REQUEST:
							EA_receive_packet = RECEIVE_MESSAGE_REQUEST;
							break;
						case MESSAGE_DELIVERY:
							EA_receive_packet = RECEIVE_MESSAGE_DELIVERY;
							break;
						default:
							cout << "ERROR: packet received unknown at time " << current_time << "\n" << endl;
							break;
					}
				}
				break;
			case RECEIVE_DATA_AV:
				if(rx.read() && sig_credit_out.read()){
					/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
					if(payload_size == 0){
						EA_receive_packet = WAIT_SEND_REQUEST;
						// cout << "AppInj: received DATA_AV" << endl;
					}
				}
				break;
			case RECEIVE_MESSAGE_REQUEST:
				if(rx.read() && sig_credit_out.read()){
					/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
					if (payload_size == 0){
						EA_receive_packet = WAIT_SEND_DELIVERY;
						// cout << "AppInj: MESSAGE_REQUEST received. Sending MESSAGE_DELIVERY" << endl;
					}
				}
				break;
			case RECEIVE_MESSAGE_DELIVERY:
				if(rx.read() && sig_credit_out.read()){
					/* For now we are ignoring the parameters: prod_task, cons_task and cons_addr */
					unsigned int data = data_in.read();
					// cout << "F" << dec << flit_counter << ": " << hex << data << endl;
					if(flit_counter == 8){
						packet_size = data;
						packet = new unsigned int[packet_size];
					} else if(flit_counter >= 13){
						packet[flit_counter - 13] = data;
					}

					if(payload_size == 0){
						switch(packet[0]){
							case APP_ALLOCATION_REQUEST:
								// cout << "AppInj: received APP_ALLOCATION_REQUEST" << endl;
								app_mapping_loader();
								EA_receive_packet = WAITING_SEND_TASK_ALLOCATION;
								break;
							case APP_MAPPING_COMPLETE:
								// cout << "AppInj: received APP_MAPPING_COMPLETE" << endl;
								EA_receive_packet = RECEIVE_MAPPING_COMPLETE;
								delete[] packet;
								packet = NULL;
								break;
							default:
								cout << "ERROR: Unknown service " << hex << packet[0] << " received at time " << dec << current_time << endl;
								break;
						}
					}
				}
				break;
			case WAIT_SEND_REQUEST:
				if(EA_send_packet == SEND_FINISHED)
					EA_receive_packet = HEADER;

				break;
			case WAIT_SEND_DELIVERY:
				if(EA_send_packet == SEND_FINISHED)
					EA_receive_packet = HEADER;

				break;
			case WAITING_SEND_TASK_ALLOCATION:
				if(EA_send_packet == SEND_FINISHED)
					EA_receive_packet = CHECK_NEXT_MAP;

				break;
			case CHECK_NEXT_MAP:
				if(pending_it != pending_allocation.end()){
					task_allocation_loader(pending_it->first, pending_it->second, 0, 0);
					pending_it++;
					EA_receive_packet = WAITING_SEND_TASK_ALLOCATION;
				} else {
					EA_receive_packet = HEADER;
				}
				break;
			case RECEIVE_MAPPING_COMPLETE:
				EA_receive_packet = HEADER;
				break;
		}

		if(rx.read() && sig_credit_out.read() && payload_size){
			payload_size--;
			flit_counter++;
		}

	}
}

/**Sequential process
 * Only is in charge to send data to NoC using as reference the packet pointer and packet_size variable.
 */
void app_injector::send_packet(){
	static FSM_send_packet last_state;
	static unsigned int aux_pkt[CONSTANT_PACKET_SIZE];
	static unsigned int aux_idx = 0;
	static unsigned int p_index = 0;

	if (reset.read() == 1)  {
		EA_send_packet = IDLE;
	} else {
		switch(EA_send_packet){
			case IDLE:
				//IDLE monitors the states of other FSM that acts as triggers
				if(EA_receive_packet == WAITING_SEND_TASK_ALLOCATION || EA_bootloader == WAIT_SEND_BOOT){
					if (credit_in.read() == 1){
						if (packet != NULL) {
							EA_send_packet = SEND_PACKET;
							p_index = 0;
						} else {
							cout << "ERROR: packet has an NULL pointer at time " << current_time <<  endl;
						}
					}
				} else if(EA_new_app_monitor == WAITING_SEND_NEW_APP){
					/* Send a packet synchronously: DATA_AV -> MESSAGE_REQUEST -> MESSAGE_DELIVERY */
					if(credit_in.read()){
						/* Can send */
						aux_idx = 0;
						aux_pkt[0] = 0x0000; 				/* Header: mapper task address */
						aux_pkt[1] = CONSTANT_PACKET_SIZE - 2; 				/* Payload size */
						aux_pkt[2] = DATA_AV;				/* Service */
						aux_pkt[3] = APP_INJECTOR_ADDRESS;	/* producer_task */
						aux_pkt[4] = 0x0000; 				/* consumer_task: mapper task id */
						aux_pkt[8] = APP_INJECTOR_ADDRESS;	/* requesting_processor */

						EA_send_packet = SEND_DATA_AV;

						// cout << "AppInj: sending DATA_AV" << endl;
					}
				} else if(EA_receive_packet == WAIT_SEND_REQUEST){
					if(credit_in.read()){
						/* Can send */
						aux_idx = 0;
						aux_pkt[0] = 0x0000; 				/* Header: mapper task address */
						aux_pkt[1] = CONSTANT_PACKET_SIZE - 2; 				/* Payload size */
						aux_pkt[2] = MESSAGE_REQUEST;		/* Service */
						aux_pkt[3] = 0x0000;				/* producer_task: mapper task id */
						aux_pkt[4] = APP_INJECTOR_ADDRESS; 	/* consumer_task */
						aux_pkt[8] = APP_INJECTOR_ADDRESS;	/* requesting_processor */

						EA_send_packet = SEND_MSG_REQUEST;

						// cout << "AppInj: sending MSG_REQUEST" << endl;
					}
				}
				break;
			case SEND_DATA_AV:
				if (credit_in.read() == 1){
					if(aux_idx < CONSTANT_PACKET_SIZE){
						tx.write(1);
						data_out.write(aux_pkt[aux_idx++]);
					} else {
						tx.write(0);
						EA_send_packet = WAIT_MESSAGE_REQUEST;
						// cout << "AppInj: waiting MESSAGE_REQUEST" << endl;
					}
				} else {
					tx.write(0);
					EA_send_packet = WAITING_CREDIT;
					last_state = SEND_DATA_AV;
				}
				break;
			case SEND_MSG_REQUEST:
				if (credit_in.read() == 1){
					if(aux_idx < CONSTANT_PACKET_SIZE){
						tx.write(1);
						data_out.write(aux_pkt[aux_idx++]);
					} else {
						tx.write(0);
						EA_send_packet = SEND_FINISHED;
					}
				} else {
					tx.write(0);
					EA_send_packet = WAITING_CREDIT;
					last_state = SEND_MSG_REQUEST;
				}
				break;
			case WAIT_MESSAGE_REQUEST:
				if(EA_receive_packet == WAIT_SEND_DELIVERY){
					if (credit_in.read() == 1){
						if (packet != NULL) {
							EA_send_packet = SEND_PACKET;
							p_index = 0;
							// cout << "AppInj: Sending packet" << endl;
						} else {
							cout << "ERROR: packet has an NULL pointer at time " << current_time <<  endl;
						}
					}
				}
				break;
			case SEND_PACKET:
				if (credit_in.read() == 1){
					if (packet_size > 0){
						tx.write(1);
						data_out.write(packet[p_index++]);
						packet_size--;
					} else {
						tx.write(0);
						EA_send_packet = SEND_FINISHED;
					}
				} else {
					tx.write(0);
					EA_send_packet = WAITING_CREDIT;
					last_state = SEND_PACKET;
				}
				break;
			case WAITING_CREDIT:
				if (credit_in.read() == 1){
					tx.write(1);
					EA_send_packet = last_state;
				}
				break;
			case SEND_FINISHED:
				if(packet != NULL){
					delete[] packet;
					packet = NULL;
				}
				EA_send_packet = IDLE;
				// cout << "AppInj: Packet sent" << endl;
				break;
		}
	}
}

void app_injector::app_mapping_loader()
{
	/* Build application mapping */
	pending_allocation.clear();
	for(unsigned int i = 1; i < packet_size; i += 2)
		pending_allocation[packet[i]] = packet[i + 1];

	delete[] packet;
	packet = NULL;

	pending_it = pending_allocation.begin();

	task_allocation_loader(pending_it->first, pending_it->second, 0, 0);
	pending_it++;
}
