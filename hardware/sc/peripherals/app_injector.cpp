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

/**
 * Monitors the file appstart.txt looking for a new app to inject on the system
 *
 * MONITORING: Reads the next 4 lines of appsstart.txt, extracting:
 * - app_name
 * - app_start_time
 * - app_task_number
 * - app_cluster_id (statically mapped cluster address)
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
	static unsigned int line_counter = 0;
	ifstream appstart_file;

	if (reset.read() == 1)  {
		EA_new_app_monitor = MONITORING;
		current_time = 0;
		app_name = "";
		app_start_time = 0;
		app_task_number = 0;
		app_cluster_id = 0;

	} else if (clock.posedge()){

		switch (EA_new_app_monitor) {

			case MONITORING: //Reads appstart.txt

				appstart_file.open("appstart.txt", std::ifstream::in);

				if (appstart_file.is_open()) {

					//Points to the last line read
					for (unsigned int i=0; i < line_counter; i++)
						getline (appstart_file,line);

					//Reads the next line of the file. Supposed to be the application name
					getline (appstart_file,app_name);

					if (app_name != "deadc0de"){

						//Gets the application start time
						getline (appstart_file,line);
						sscanf( line.substr(0, 8).c_str(), "%u", &app_start_time ); //Start time is in milliseconds

						//Gets the application cluster
						getline (appstart_file,line);
						sscanf( line.substr(0, 8).c_str(), "%d", &app_cluster_id );

						//Gets the application task number
						getline (appstart_file,line);
						sscanf( line.substr(0, 8).c_str(), "%u", &app_task_number );

						line_counter = line_counter + 4;

						task_static_mapping = new int[app_task_number];

						//cout << "App name: " << app_name << endl;
						//cout << "app_cluster_id: " << app_cluster_id << endl;
						//cout << "app_task_number: " << app_task_number << endl;

						//Gets the allocated processor for each task, useful for static task mapping
						for(unsigned int i=0; i<app_task_number; i++){
							//Gets the application task number
							getline (appstart_file,line);
							sscanf( line.substr(0, 8).c_str(), "%d", &task_static_mapping[i]);
							//cout << "task id " << i << " mapped at " <<  task_static_mapping[i] << endl;
							line_counter++;
						}

						cout << "App Injector requesting app " << app_name << endl;

						EA_new_app_monitor = WAITING_TIME;
					}

					appstart_file.close();

				} else {
					cout << "Unable to open file appstart.txt" << endl;
				}

				break;

			case WAITING_TIME: //Test when the current time reach the application start time

				if ( (app_start_time * 100000) <= current_time ){

					packet_size = CONSTANT_PACKET_SIZE;

					packet = new unsigned int[CONSTANT_PACKET_SIZE];

					//Asembles the packet
					packet[0] = MPE_ADDR;
					packet[1] = CONSTANT_PACKET_SIZE-2;
					packet[2] = NEW_APP_REQ;
					packet[4] = app_cluster_id;
					packet[8] = app_task_number;

					EA_new_app_monitor = WAITING_SEND_APP_REQ;
				}

				break;

			case WAITING_SEND_APP_REQ:

				if (EA_send_packet == SEND_FINISHED)
					EA_new_app_monitor = IDLE_MONITOR;

				break;
			case IDLE_MONITOR:

				//Waits until the last task allocation packet be sent before to continue monitoring
				if (EA_receive_packet == WAITING_SEND_TASK_ALLOCATION && EA_send_packet == SEND_FINISHED)
					EA_new_app_monitor = MONITORING;

				break;
		}

		current_time++;
	}
}

/**Reads application repository and fills the packet variable to sent it through the NoC.
 * Assembles a set of TASK_ALLOCATION packet according to the number of application's tasks
 * and the informations of tasks_info pointer. tasks_info pointer is filled during the manipulation
 * of a APP_ALLOCATION packet inside the receive_packet function.
 * After finish this function the receive_packet function signals to the send_packet function to start
 * the packet transmition through the NoC.
 * This function MODIFIES the following global variables:
 * - *packet = receives a pointer to the allocated memory block
 * - packet_size = receives the packet size
 *
 * This function USES the following global variables:
 * - app_name = used to form the application repository's path
 * - app_task_number = used to compute the repository's size
 * - tasks_info = used to gather information about application tasks (information provided during APP_ALLOCATION)
 * - cluster_address = address of the task manager (information provided during RECEIVE_APP_ACK)
 */
void app_injector::task_allocation_loader(){
	string line;
	string path = "../applications/" + app_name + "/repository.txt";
	ifstream repo_file (path.c_str());
	int ptr_index;
	int descriptor_size;
	unsigned int id, allocated_proc, code_size;

	if (repo_file.is_open()) {

		descriptor_size = (TASK_DESCRIPTOR_SIZE * app_task_number) + 1;

		//cout << "\n\n------ Task allocation loader ---- \nDescriptor size: " << descriptor_size << endl;

		for(int i=0; i<descriptor_size; i++)
			getline (repo_file,line); //Jumps app descriptor

		packet_size = 0;
		for(unsigned int i = 0; i < app_task_number; i++)
			packet_size += CONSTANT_PACKET_SIZE +  tasks_info[3+(i*4)]; //Gets code size for each task

		packet = new unsigned int[packet_size];

		ptr_index = 0;
		for(unsigned int task_id = 0; task_id<app_task_number; task_id++){

			id 				= tasks_info[0+(task_id*4)]; //Gets task id
			allocated_proc 	= tasks_info[1+(task_id*4)]; //Gets task allocated proc
			//address  		= tasks_info[2+(task_id*4)]; //Gets task txt address
			code_size 		= tasks_info[3+(task_id*4)]; //Gets task txt size

			/*cout << "\n\n****************\nNew task id: " << id << endl;
			cout << "allocated_proc: " << allocated_proc << endl;
			cout << "address: " << address << endl;
			cout << "code_size: " << code_size << endl;*/

			//Assembles ServiceHeader
			packet[ptr_index++] = allocated_proc; //Header <- Gets allocated proc
			packet[ptr_index++] = (CONSTANT_PACKET_SIZE - 2) + code_size; //Assemble payload size
			packet[ptr_index++] = TASK_ALLOCATION; //Packet service
			packet[ptr_index++] = id;
			packet[ptr_index++] = cluster_address; //Master ID
			ptr_index 			= ptr_index + 5; //Jumps to code_size field on ServiceHeader
			packet[ptr_index++] = code_size; //Code size
			ptr_index 			= ptr_index + 2; //Jumps to the end of ServiceHeader

			//Assembles txt
			for(unsigned int i=0; i<code_size; i++){
				getline (repo_file,line);
				sscanf( line.substr(0, 8).c_str(), "%x", &packet[ptr_index++]);
			}
		}

		repo_file.close();
	} else {
		cout << "Unable to open file " << path << endl;
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
	string path = "../applications/" + app_name + "/repository.txt";
	ifstream repo_file (path.c_str());
	int file_length;
	int ptr_index;
	int allocated_proc_index;
	int task_index;

	file_length = 0;
	ptr_index = 0;

	if (repo_file.is_open()) {

		file_length = (TASK_DESCRIPTOR_SIZE * app_task_number) + 1; //Plus one because the descriptors stores the information of task number at the firt line

		// Points to the begging of file
		repo_file.clear();
		repo_file.seekg (0, repo_file.beg);

		//Sets the NoC's packet size
		packet_size = CONSTANT_PACKET_SIZE + file_length;

		//Allocate memory
		packet = new unsigned int[packet_size];

		//Assembles the Service Header on packet
		packet[0] = cluster_address; // Manager address
		packet[1] = packet_size - 2; // Packet payload
		packet[2] = NEW_APP; //Packet service
		packet[3] = app_id; //Packet service
		packet[8] = file_length; // App descriptor size

		ptr_index = CONSTANT_PACKET_SIZE; //ptr_index starts after ServiceHeader

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

		/*for(int i=0; i<packet_size; i++){
			cout << hex << packet[i] << endl;
		}*/

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
 * If service is APP_ALLOCATION_REQUEST goes to RECEIVE_APP_ALLOCATION state.

 * RECEIVE_APP_ACK - Extracts the manager address where the application was mapped, in sequence, loads the application description
 * from repository file by calling function "app_descriptor_loader". Such function creates a block of memory that store all relevant
 * information about the application and that need to be sent to the manager (which the application was mapped) through a NEW_APP packet.
 *
 * RECEIVE_APP_ALLOCATION - Extracts the application task number and 4 relevant information about the application task, which are
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

	/*static unsigned int payload_size = 0;
	static unsigned int flit_counter = 0;*/
	//static unsigned int task_info_index = 0;
	static unsigned int task_info_size = 0;

	if (reset.read() == 1)  {
		EA_receive_packet = HEADER;
		task_info_size = 0;
		task_info_index = 0;
		sig_credit_out.write(1);
	} else {

		switch (EA_receive_packet) {

			case HEADER:

				if (rx.read() == 1)
					EA_receive_packet = PAYLOAD_SIZE;

				break;

			case PAYLOAD_SIZE:

				if (rx.read() == 1){
					payload_size = data_in.read();
					flit_counter = 2;
					EA_receive_packet = SERVICE;
				}

				break;
			case SERVICE:

				if (rx.read() == 1){
					if (data_in.read() == APP_REQ_ACK)
						EA_receive_packet = RECEIVE_APP_ACK;
					else if (data_in.read() == APP_ALLOCATION_REQUEST)
						EA_receive_packet = RECEIVE_APP_ALLOCATION;
				}

				break;

			case RECEIVE_APP_ACK: //APP_REQ_ACK:

				if (rx.read() == 1){

					if(flit_counter == 4){ //Reads app_ID flit from ServiceHeader

						app_id = data_in.read();

					} else if (flit_counter == 5){///Reads cluster_ID flit from ServiceHeader

						cluster_address = data_in.read();

						cout << "Manager sent ACK" << endl;
						//Loads app descriptor to the pointer * packet (used in send_packet function)
						app_descriptor_loader();

					}

					if (payload_size == 0)
						EA_receive_packet = WAITING_SEND_NEW_APP;
				}

				break;

			case RECEIVE_APP_ALLOCATION:

				if (rx.read() == 1){

					if (flit_counter == 9){// Gets task number from flit 9

						//Vezes 4 porque o mestre envia 4 informações de cada tarefa
						task_info_size = data_in.read() * 4;

						//cout << "Received task number " << task_info_size/4 << endl;

						tasks_info = new unsigned int[task_info_size];

						task_info_index = 0;

						if (tasks_info == NULL)
							cout << "ERROR: tasks_info is NULL" << endl;

					} else if (flit_counter > CONSTANT_PACKET_SIZE){ //Reads task's info from packet

						tasks_info[task_info_index++] = data_in.read();

						if (task_info_size == 0 && payload_size != 0)
							cout << "\nERROR: Packet APP_ALLOCATION is bigger than expected\n" << endl;
						else
							task_info_size--;
					}

					if (payload_size == 0){

						task_allocation_loader();

						delete[] tasks_info;

						EA_receive_packet = WAITING_SEND_TASK_ALLOCATION;
					}
				}

				break;

			case WAITING_SEND_NEW_APP:
				if (EA_send_packet == SEND_FINISHED){
					EA_receive_packet = HEADER;
					sig_credit_out.write(1);
				} else
					sig_credit_out.write(0);
				break;

			case WAITING_SEND_TASK_ALLOCATION:
				if (EA_send_packet == SEND_FINISHED){
					EA_receive_packet = HEADER;
					sig_credit_out.write(1);
				} else
					sig_credit_out.write(0);
				break;
		}//end switch

		if (rx.read() == 1 && sig_credit_out.read() == 1 && payload_size != 0){
			payload_size--;
			flit_counter++;
		}

	}//end else
}

/**Sequential process
 * Only is in charge to send data to NoC using as reference the packet pointer and packet_size variable.
 */
void app_injector::send_packet(){

	static unsigned int p_index = 0;

	if (reset.read() == 1)  {
		EA_send_packet = IDLE;
	} else {

		switch (EA_send_packet) {

			case IDLE:
				if (EA_new_app_monitor == WAITING_SEND_APP_REQ || EA_receive_packet == WAITING_SEND_NEW_APP || EA_receive_packet == WAITING_SEND_TASK_ALLOCATION) {
					if (packet != NULL) {
						EA_send_packet = SEND_PACKET;
						p_index = 0;
					} else
						cout << "ERROR: packet has an NULL pointer" << endl;
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
				}

				break;

			case WAITING_CREDIT:
				if (credit_in.read() == 1){
					tx.write(1);
					EA_send_packet = SEND_PACKET;
				}
				break;

			case SEND_FINISHED:

				delete[] packet;

				EA_send_packet = IDLE;

				break;
		}
	}
}
