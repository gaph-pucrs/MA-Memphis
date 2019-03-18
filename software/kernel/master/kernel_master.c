/*!\file kernel_master.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * Kernel master is the system manager kernel.
 *
 * \detailed
 * kernel_master is the core of the OS running into the managers processors (local and global).
 * It assumes two operation modes: global or local. The operation modes is defined by the global variable is_global_master.
 * Local operation mode: runs into local managers. Manage the applications and task mapping.
 * Global operation mode: runs into global manager. Runs all functions of the local operation mode, further the applications admission control.
 * The kernel_master file uses several modules that implement specific functions
 */

#include "kernel_master.h"

#include "../../modules/utils.h"
#include "../../include/plasma.h"
#include "../../include/services.h"
#include "../../modules/packet.h"
#include "../../modules/resource_manager.h"
#include "../../modules/reclustering.h"
#include "../../modules/applications.h"
#include "../../modules/processors.h"


/*Local Manager (LM) global variables*/
unsigned int 	pending_app_to_map = 0; 					//!< Controls the number of pending applications already handled by not completely mapped
unsigned char 	is_global_master;							//!< Defines if this kernel is at global or local operation mode
unsigned int 	global_master_address;						//!< Used to stores the global master address, is useful in local operation mode
unsigned int 	terminated_task_master[MAX_TASKS_APP];		//!< Auxiliary array that stores the terminated task list


/*Global Master (GM) global variables*/
unsigned int 	total_mpsoc_resources = (MAX_LOCAL_TASKS * MAX_SLAVE_PROCESSORS);	//!< Controls the number of total slave processors pages available. Is the admission control variable
unsigned int 	terminated_app_count = 0;											//!< Used to fires the END OF ALL APPLIATIONS
unsigned int 	waiting_app_allocation = 0;											//!< Signal that an application is not fully mapped


/** Receive a address and return the cluster index of array cluster_info[]
 *  \param x Address x of the manager processor
 *  \param y Address y of the manager processor
 *  \return Index of array cluster_info[]
 */
int get_cluster_ID(int x, int y){

	for (int i=0; i<CLUSTER_NUMBER; i++){
		if (cluster_info[i].master_x == x && cluster_info[i].master_y == y){
			return i;
		}
	}
	puts("ERROR - cluster nao encontrado\n");
	return -1;
}

/** Receives a slave address and tells which is its master address
 *  \param slave_address The XY slave address
 *  \return The master address of the slave. Return -1 if there is no master (ERROR situation)
 */
int get_master_address(int slave_address){

	ClusterInfo *cf;
	int proc_x, proc_y;

	proc_x = slave_address >> 8;
	proc_y = slave_address & 0xFF;

	for(int i=0; i<CLUSTER_NUMBER; i++){
		cf = &cluster_info[i];
		if (cf->xi <= proc_x && cf->xf >= proc_x && cf->yi <= proc_y && cf->yf >= proc_y){
			return get_cluster_proc(i);
		}
	}

	puts("ERROR: no master address found\n");
	while(1);
	return -1;
}

/** Assembles and sends a APP_TERMINATED packet to the global master
 *  \param app The Applications address
 *  \param terminated_task_list The terminated task list of the application
 */
void send_app_terminated(Application *app, unsigned int * terminated_task_list){

	ServiceHeader *p = get_service_header_slot();

	p->header = global_master_address;

	p->service = APP_TERMINATED;

	p->app_ID = app->app_ID;

	p->app_task_number = app->tasks_number;

	send_packet(p, (unsigned int) terminated_task_list, app->tasks_number);

	while (MemoryRead(DMNI_SEND_ACTIVE));

}

/** Assembles and sends a TASK_RELEASE packet to a slave kernel
 *  \param app The Application instance
 */
void send_task_release(Application * app){

	ServiceHeader *p;
	unsigned int app_tasks_location[app->tasks_number];

	for (int i =0; i<app->tasks_number; i++){
		app_tasks_location[i] = app->tasks[i].allocated_proc;
	}

	for (int i =0; i<app->tasks_number; i++){

		p = get_service_header_slot();

		p->header = app->tasks[i].allocated_proc;

		p->service = TASK_RELEASE;

		p->app_task_number = app->tasks_number;

		p->task_ID = app->tasks[i].id;

		p->data_size = app->tasks[i].data_size;

		p->bss_size = app->tasks[i].bss_size;

		send_packet(p, (unsigned int) app_tasks_location, app->tasks_number);

		app->tasks[i].status = TASK_RUNNING;

		putsv("\n -> send TASK_RELEASE to task ", p->task_ID);
		//puts(" in proc "); puts(itoh(p->header)); puts("\n----\n");
	}

	app->status = RUNNING;

	//Necessary because the DMNI uses data pointers created inside stack. If the function exit and DMNI still sending the packet, the data can be loosed.
	while(MemoryRead(DMNI_SEND_ACTIVE));
}

/** Assembles and sends a APP_ALLOCATION_REQUEST packet to the global master
 *  \param app The Application instance
 *  \param task_info An array containing relevant task informations
 */
void send_app_allocation_request(Application * app, unsigned int * task_info){

	ServiceHeader *p;

	p = get_service_header_slot();

	p->header = APP_INJECTOR;

	p->service = APP_ALLOCATION_REQUEST;

	p->master_ID = net_address;

	p->app_task_number = app->tasks_number;

	putsv("Send new APP REQUEST to master - id: ", p->task_ID );
	putsv(" app id: ", app->app_ID);
	putsv(" manager ID: ", p->master_ID);

	send_packet(p, (unsigned int) task_info, app->tasks_number*4);

	if (!is_global_master) {

		//Sends task mapping report to global manager enabling it to keep updated the cluster usage even in case of reclustering
		p = get_service_header_slot();

		p->header = global_master_address;

		p->service = TASK_MAPPING_REPORT;

		p->master_ID = net_address;

		p->app_task_number = app->tasks_number;

		puts("Send TASK MAPPING REPORT to global manager\n");

		send_packet(p, (unsigned int) task_info, app->tasks_number*4);

	}

	while (MemoryRead(DMNI_SEND_ACTIVE));

}

/** Assembles and sends a TASK_MIGRATION packet to a slave kernel
 *  \param task_ID The task ID to be migrated
 *  \param new_proc The new processor address of task_ID
 */
int send_task_migration(int task_ID, int new_proc){


	int old_proc;
	Application * app;

	app = get_application_ptr(task_ID >> 8);

	if (app->status != RUNNING){
		putsvsv("Warning! :: Task migration for task ", task_ID, " refused, is not running app ", (task_ID >> 8));
		return 0;
	}

	ServiceHeader *p = get_service_header_slot();

	old_proc = get_task_location(task_ID);

	p->header = old_proc;

	p->service = TASK_MIGRATION;

	p->task_ID = task_ID;

	p->allocated_processor = new_proc;

	send_packet(p, 0, 0);

	//Update task status
	set_task_migrating(task_ID);

	putsvsv("Task migration order of task ", task_ID, " to proc ", old_proc);

	return 1;

}

/** Requests a new application to the global master kernel
 *  \param app Application to be requested
 */
void request_application(Application *app){

	Task *t;
	unsigned int task_info[app->tasks_number*4];
	int index_counter;

	puts("\nRequest APP\n");

	pending_app_to_map--;

	index_counter = 0;

	for (int i=0; i<app->tasks_number; i++){

		t = &app->tasks[i];

		if (t->allocated_proc == -1){
			putsv("ERROR task id not allocated: ", t->id);
			while(1);
		}

		t->status = REQUESTED;

		task_info[index_counter++] = t->id;
		task_info[index_counter++] = t->allocated_proc;
		task_info[index_counter++] = t->initial_address;
		task_info[index_counter++] = t->code_size;
	}

	//Sends app allocation to Injector
	send_app_allocation_request(app, task_info);

}

/** Handles a pending application. A pending application it the one which there is some task waiting to be mapped by reclustering protocol
 */
void handle_pending_application(){

	Application *app = 0;
	int request_app = 0;

	puts("Handle next application \n");

	/*Selects an application pending to be mapped due reclustering*/
	app = get_next_pending_app();

	//This line fires the reclustering protocol
	request_app = reclustering_next_task(app);

	if (request_app){

		app->status = READY_TO_LOAD;

		request_application(app);
	}
}

/** Handles an application which terminated its execution
 *  \param appID Application ID of the terminated app
 *  \param app_task_number Application task number
 *  \param app_master_addr Application master XY address
 */
void handle_app_terminated(int appID, unsigned int app_task_number, unsigned int app_master_addr){

	unsigned int task_master_addr;
	int borrowed_cluster, original_cluster;

	//putsv("\n --- > Handle APP terminated- app ID: ", appID);
	//puts("original master addrr "); puts(itoh(app_master_addr)); puts("\n");

	original_cluster = get_cluster_ID(app_master_addr >> 8, app_master_addr & 0xFF);

	for (int i=0; i<app_task_number; i++){
		task_master_addr = terminated_task_master[i];

		//puts("Terminated task id "); puts(itoa(appID << 8 | i)); puts(" with physycal master addr "); puts(itoh(task_master_addr)); puts("\n");

		if (task_master_addr != app_master_addr && task_master_addr != net_address){

			borrowed_cluster = get_cluster_ID(task_master_addr >> 8, task_master_addr & 0xFF);

			release_cluster_resources(borrowed_cluster, 1);

		} else if (task_master_addr != net_address){ // Because only the global calls this funtion
			//puts("Remove original\n");
			release_cluster_resources(original_cluster, 1);
		}
	}
	total_mpsoc_resources += app_task_number;
	terminated_app_count++;

	//puts("\n-------\n");

	/*if (terminated_app_count == APP_NUMBER){
		puts("FINISH ");puts(itoa(MemoryRead(TICK_COUNTER))); puts("\n");
		MemoryWrite(END_SIM,1);
	}*/

}

/** Handles a new packet from NoC
 */
void handle_packet() {

	int app_id, allocated_tasks, index_counter, master_addr, task_id, allocated_proc;
	volatile ServiceHeader p;
	Application *app;
	unsigned int task_info[MAX_TASKS_APP*4];

	read_packet((ServiceHeader *)&p);

	switch (p.service){

	case NEW_APP:

		handle_new_app(p.app_ID, 0, p.app_descriptor_size);

		break;

	case TASK_ALLOCATED:

		putsv("\n -> TASK ALLOCATED from task ", p.task_ID);
		app_id = p.task_ID >> 8;

		app = get_application_ptr(app_id);

		allocated_tasks = set_task_allocated(app, p.task_ID);

		putsv("Allocated tasks: ", allocated_tasks);

		if (allocated_tasks == app->tasks_number){

			send_task_release(app);
		}

		break;

	case INITIALIZE_CLUSTER:

		global_master_address = p.source_PE;

		reclustering_setup(p.cluster_ID);

		initialize_slaves();

		break;

	case LOAN_PROCESSOR_REQUEST:
	case LOAN_PROCESSOR_DELIVERY:
	case LOAN_PROCESSOR_RELEASE:

		handle_reclustering((ServiceHeader *)&p);

		break;

	case TASK_MAPPING_REPORT:

		//puts("New app allocation request\n");

		index_counter = 0;

		DMNI_read_data((unsigned int)task_info, p.app_task_number*4);

		for(int i=0; i< p.app_task_number; i++){

			task_id = task_info[index_counter++];
			allocated_proc = task_info[index_counter++];
			index_counter++; //Skips address
			index_counter++; //Skips size

			puts("New task mapping report: "); puts(itoa(task_id)); puts(" mapped at proc ");
			puts(itoh(allocated_proc)); puts("\n");

			/*These lines above mantain the cluster resource control at a global master perspective*/
			master_addr = get_master_address(allocated_proc);

			//net_address is equal to global master address, it is necessary to verifies if is master because the master controls the cluster resources by gte insertion of new tasks request
			if (master_addr != net_address){

				//Reuse of the variable master_addr to store the cluster ID
				master_addr = get_cluster_ID(master_addr >> 8, master_addr & 0xFF);

				putsv("New resource allocated to cluster ", master_addr);
				allocate_cluster_resource(master_addr, 1);
			}

		}

		break;

	case APP_TERMINATED:

		DMNI_read_data((unsigned int)terminated_task_master, p.app_task_number);

		handle_app_terminated(p.app_ID, p.app_task_number, p.source_PE);

		break;

	case TASK_TERMINATED:

		app_id = p.task_ID >> 8;

		app = get_application_ptr(app_id);

		set_task_terminated(app, p.task_ID);

		if (p.master_ID == net_address){
			page_released(clusterID, p.source_PE, p.task_ID);
		}

		//Test if is necessary to terminate the app
		if (app->terminated_tasks == app->tasks_number){

			for (int i=0; i<app->tasks_number; i++){

				if (app->tasks[i].borrowed_master != -1){
					terminated_task_master[i] = app->tasks[i].borrowed_master;
				} else {
					terminated_task_master[i] = net_address;
				}
			}

			if (is_global_master) {

				handle_app_terminated(app->app_ID, app->tasks_number, net_address);

			} else {

				send_app_terminated(app, terminated_task_master);
			}

			remove_application(app->app_ID);
		}

		break;

	case TASK_TERMINATED_OTHER_CLUSTER:

		page_released(clusterID, p.source_PE, p.task_ID);

		break;

	case TASK_MIGRATED:

		putsvsv("Received task migrated - task id: ", p.task_ID, " new proc ", p.source_PE);

		set_task_migrated(p.task_ID, p.source_PE);

		//Update the page
		page_released(clusterID, p.released_proc, p.task_ID);
		page_used(clusterID, p.source_PE, p.task_ID);

		break;

	case SLACK_TIME_REPORT:

		update_proc_slack_time(p.source_PE, p.cpu_slack_time);

		break;

	case NEW_APP_REQ: //Packet from Injector containing the number of task of the new requested app

		if (waiting_app_allocation){
			puts("ERROR: Waiting app allocation\n");
			while(1);
		}

		if (p.cluster_ID == -1)
			master_addr = CLUSTER_NUMBER; //Receives a invalid cluster address to signal that cluster mapping must be dynamic
		else
			master_addr = p.cluster_ID;

		//If 0 then there is no application to be allocated, otherwise, stores the number of application tasks and cluster ID
		waiting_app_allocation = p.app_task_number << 16 | master_addr;

		puts("-----> NEW_APP_REQ from app injector. Task number is "); puts(itoa(p.app_task_number)); putsv(". App mapped at cluster ", p.cluster_ID);

		break;

	default:
		puts("ERROR: service unknown ");puts(itoh(p.service)); puts("\n");
		putsv("Time: ", MemoryRead(TICK_COUNTER));
		break;
	}

}


/** Initializes all slave processor by sending a INITIALIZE_SLAVE packet to each one
 */
void initialize_slaves(){

	ServiceHeader *p;
	int proc_address, index_counter;

	init_procesors();

	index_counter = 0;

	for(int i=cluster_info[clusterID].xi; i<=cluster_info[clusterID].xf; i++) {

		for(int j=cluster_info[clusterID].yi; j<=cluster_info[clusterID].yf; j++) {

			proc_address = i*256 + j;//Forms the proc address

			if( proc_address != net_address) {

				//Fills the struct processors
				add_procesor(proc_address);

				//Sends a packet to the slave
				p = get_service_header_slot();

				p->header = proc_address;

				p->service = INITIALIZE_SLAVE;

				send_packet(p, 0, 0);

				index_counter++;
			}
		}
	}

}

/** Initializes all local managers by sending a INITIALIZE_CLUSTER packet to each one
 */
void initialize_clusters(){

	int cluster_master_address;
	ServiceHeader *p;

	for(int i=0; i<CLUSTER_NUMBER; i++) {

		cluster_master_address = (cluster_info[i].master_x << 8) | cluster_info[i].master_y;

		puts("Initializing cluster address "); puts(itoa(cluster_info[i].master_x)); puts("x"); puts(itoa(cluster_info[i].master_y)); puts("\n");

		if(cluster_master_address != global_master_address){

			p = get_service_header_slot();

			p->header = cluster_master_address;

			p->service = INITIALIZE_CLUSTER;

			p->source_PE = global_master_address;

			p->cluster_ID = i;

			send_packet(p, 0, 0);

		} else {

			reclustering_setup(i);

		}
	}

	puts("Global manager initialized!\n\n");
}

/** Handles a new application incoming from the global manager or by repository
 * \param app_ID Application ID to be handled
 * \param ref_address Pointer to the application descriptor. It can point to a array (local manager) or the repository directly (global manager)
 * \param app_descriptor_size Size of the application descriptor
 */
void handle_new_app(int app_ID, volatile unsigned int *ref_address, unsigned int app_descriptor_size){

	Application *application;
	int mapping_completed = 0;

	putsv("Descriptor size: ", app_descriptor_size);
	putsv("App ID ", app_ID);

	//Cuidado com app_descriptor_size muito grande, pode estourar a memoria
	unsigned int app_descriptor[app_descriptor_size];

	puts("programming DMNI\n");

	DMNI_read_data( (unsigned int) app_descriptor, app_descriptor_size);

	ref_address = app_descriptor;

	puts("DNNI reading complete\n");

	//Creates a new app by reading from ref_address
	application = read_and_create_application(app_ID, ref_address);

	pending_app_to_map++;

	mapping_completed = application_mapping(clusterID, application->app_ID);

	if (mapping_completed){

		application->status = READY_TO_LOAD;

		request_application(application);

	} else {

		puts("Application waiting reclustering\n");

		application->status = WAITING_RECLUSTERING;

	}
}

/** Handles a new application request triggered by App Injector.
 */
void handle_app_request(){

	unsigned int num_app_tasks, selected_cluster_proc;
	static unsigned int app_id_counter = 0;
	int selected_cluster, cluster_static;
	ServiceHeader *p;

	num_app_tasks = waiting_app_allocation >> 16;

	if (num_app_tasks > total_mpsoc_resources){
		puts("Cluster full\n");
		return;
	}

	cluster_static = waiting_app_allocation & 0xFFFF;

	puts("\nNew app req handled! - cluster mapping is ");

	if (cluster_static >= CLUSTER_NUMBER){

		puts("dynamic\n");

		selected_cluster = SearchCluster(clusterID, num_app_tasks);

	} else {
		puts("static\n");
		selected_cluster = cluster_static;
	}

	waiting_app_allocation = 0;

	putsv("\nApplication ID: ", app_id_counter);

	//putsv("Global Master reserve application: ", num_app_tasks);
	//putsv("total_mpsoc_resources ", total_mpsoc_resources);

	total_mpsoc_resources -= num_app_tasks;

	selected_cluster_proc = get_cluster_proc(selected_cluster);

	putsv("Selected cluster: ", selected_cluster_proc);

	//Send ACK packet to App Injector
	p = get_service_header_slot();

	p->header = APP_INJECTOR;

	p->service = APP_REQ_ACK;

	p->cluster_ID = selected_cluster_proc;

	p->app_ID = app_id_counter;

	send_packet(p, 0, 0);

	app_id_counter++;
}


int main() {

	//By default Memphis assumes that GM is positioned at address 0
	if ( MemoryRead(NI_CONFIG) == 0){

		puts("This kernel is global master\n");

		is_global_master = 1;

		global_master_address = net_address;

		initialize_clusters();

		initialize_slaves();

	} else {

		puts("This kernel is local master\n");

		is_global_master = 0;
	}

	initialize_applications();

	init_service_header_slots();

	puts("Kernel Initialized\n");


	//Eternal Manager looping
	for (;;) {

		if (noc_interruption){

			handle_packet();

		} else if (pending_app_to_map && is_reclustering_NOT_active()){

			handle_pending_application();

		} else if (waiting_app_allocation && !MemoryRead(DMNI_SEND_ACTIVE)) {

			handle_app_request();
		}
	}

	return 0;
}
