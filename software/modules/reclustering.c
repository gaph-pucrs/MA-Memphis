/*!\file reclustering.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to reclustering
 * This module is used by the kernel manager
 * \detailed
 * Reclustering is a functionality that enable the system to borrow some resources (free pages) of a cluster to another cluster
 */


#include "reclustering.h"
#include "../include/services.h"
#include "utils.h"
#include "resource_manager.h"
#include "processors.h"
#include "packet.h"

#define RECLUSTERING_DEBUG	0		//!<When enable compile the puts in this file

Reclustering reclustering;			//!<Reclustering structure instance

//Max o levels of MPSoCs
int max_neighbors_level = 0;		//!<Max of neighbors clusters for each reclustering level

//Cluster dimmensions
unsigned int clusterID;					//!<Current cluster ID
unsigned int starting_x_cluster_size;	//!<Starting X size of cluster when reclustering is started
unsigned int starting_y_cluster_size;	//!<Starting Y size of cluster when reclustering is started

/**Setup a reclustering, called by kernel and configure the cluster ID and initializes some variables
 * \param c_id Current cluster ID
 */
void reclustering_setup(int c_id){

	clusterID = c_id;

	starting_x_cluster_size = cluster_info[clusterID].xf - cluster_info[clusterID].xi + 1;

	starting_y_cluster_size = cluster_info[clusterID].yf - cluster_info[clusterID].yi + 1;

#if RECLUSTERING_DEBUG
	putsv("CLuster id: ", clusterID);
#endif

	reclustering.active = 0;

	if ((XDIMENSION/XCLUSTER) > (YDIMENSION/YCLUSTER)) {
		max_neighbors_level = (XDIMENSION/XCLUSTER) - 1;
	} else {
		max_neighbors_level = (YDIMENSION/YCLUSTER) - 1;
	}
}

/**Test if the reclustering is not active
 * \return 1 if is not active, 0 if is active
 */
int is_reclustering_NOT_active(){
	return !reclustering.active;
}

/**Assembles and sends a LOAN_PROCESSOR_REQUEST packet to the neighbor master kernel
 * \param address Neighbor master address
 * \param taskID Task ID target of reclustering
 */
void send_loan_proc_request(int address, int taskID){

	ServiceHeader *p = get_service_header_slot();

	p->header = address;

	p->service = LOAN_PROCESSOR_REQUEST;

	p->task_ID = taskID;

	p->allocated_processor = (cluster_info[clusterID].master_x << 8) | cluster_info[clusterID].master_y;

	send_packet(p, 0, 0);

#if RECLUSTERING_DEBUG
	puts("-> send loan proc REQUEST para proc "); puts(itoh(address)); putsv(" task id ", taskID);
#endif

}

/**Assembles and sends a LOAN_PROCESSOR_DELIVERY packet to the target of reclustering (requesting) master kernel
 * \param master_address Requesting reclustering master address
 * \param taskID Task ID target of reclustering
 * \param proc Allocated processor
 * \param hops Number of hops from the target processor
 */
void send_loan_proc_delivery(int master_address, int taskID, int proc, int hops){

	ServiceHeader *p = get_service_header_slot();

	p->header = master_address;

	p->service = LOAN_PROCESSOR_DELIVERY;

	p->task_ID = taskID;

	p->hops = hops;

	p->allocated_processor = proc;

	send_packet(p, 0, 0);

#if RECLUSTERING_DEBUG
	puts("-> send loan proc DELIVERY para proc "); puts(itoh(master_address)); putsv(" task id ", taskID);
#endif

}

/**Assembles and sends a LOAN_PROCESSOR_RELEASE packet to the neighbor master kernel
 * \param master_address Neighbor master address
 * \param release_proc Address of the released processor
 * \param taskID Task ID target of reclustering
 */
void send_loan_proc_release(int master_address, int release_proc, int taskID){

	ServiceHeader *p = get_service_header_slot();

	p->header = master_address;

	p->service = LOAN_PROCESSOR_RELEASE;

	p->task_ID = taskID;

	p->released_proc = release_proc;

	send_packet(p, 0, 0);

#if RECLUSTERING_DEBUG
	puts("-> send loan proc RELEASE para proc "); puts(itoh(master_address)); puts(" releasing the proc "); puts(itoh(release_proc)); puts("\n");
#endif

}

/**Fires a new round of the reclustering protocol. This function send the load request to the neighboors masters
 * delimited by the reclustering.neighbors_level variable
 */
void fires_reclustering_protocol(){

	int other_x, other_y;
	int this_x, this_y;
	int hops_x, hops_y;
	int cluster_x_size, initial_x_level;
	int cluster_y_size, initial_y_level;
	int other_address;

#if RECLUSTERING_DEBUG
	putsv(" Fires reclustering protocol - level ", reclustering.neighbors_level);
#endif

	reclustering.pending_loan_delivery = 0;
	reclustering.current_borrowed_proc = -1;
	reclustering.current_borrowed_master = -1;
	reclustering.min_loan_proc_hops = 0xFFFFFF;

	cluster_x_size = starting_x_cluster_size; //+ 1 para calcular o numero de processadores do eixo x e nao o numero de hops
	cluster_y_size = starting_y_cluster_size;

	putsv("starting_x_cluster_size: ", starting_x_cluster_size);
	putsv("starting_y_cluster_size: ", starting_y_cluster_size);

	initial_x_level = cluster_x_size * (reclustering.neighbors_level -1);
	initial_y_level = cluster_y_size * (reclustering.neighbors_level -1);

	cluster_x_size *= reclustering.neighbors_level;
	cluster_y_size *= reclustering.neighbors_level;

	this_x = cluster_info[clusterID].master_x;
	this_y = cluster_info[clusterID].master_y;

	for(int i=0; i<CLUSTER_NUMBER; i++) {

		other_x = cluster_info[i].master_x;
		other_y = cluster_info[i].master_y;

		if (clusterID == i){
			continue;
		}

		hops_x = abs(this_x - other_x);
		hops_y = abs(this_y - other_y);

		if ( (hops_x > initial_x_level || hops_y > initial_y_level) &&
				hops_x <= cluster_x_size && hops_y <= cluster_y_size) {

			other_address = (other_x << 8) | other_y;

			send_loan_proc_request(other_address, reclustering.task->id);

			reclustering.pending_loan_delivery++;
		}
	}
}

/**handle reclustering packets incoming from kernel master.
 * The kernel master kwnows that the packet is a reclustering related packet and
 * calls this function passing the ServiceHeader pointer.
 * \param p ServiceHeader pointer
 */
void handle_reclustering(ServiceHeader * p){

	int mapped_proc;
	Application *app;
	int hops;

	switch(p->service){

	case LOAN_PROCESSOR_REQUEST:

#if RECLUSTERING_DEBUG
		puts("\nReceive LOAN_PROCESSOR_REQUEST "); puts(" from proc "); puts(itoh(p->source_PE)); puts("\n");
#endif

		if (cluster_info[clusterID].free_resources <= 0){

			send_loan_proc_delivery(p->source_PE, p->task_ID, -1, -1);

		} else {

			//Procura pelo processador mais proximo do processador requisitnate
			mapped_proc = map_task(p->task_ID);

			hops = (p->allocated_processor >> 8) + (mapped_proc >> 8);
			hops += (p->allocated_processor & 0xFF) + (mapped_proc & 0xFF);

#if RECLUSTERING_DEBUG
			puts("Alocou proc "); puts(itoh(mapped_proc)); puts("\n");
#endif

			send_loan_proc_delivery(p->source_PE, p->task_ID, mapped_proc, hops);

			page_used(clusterID, mapped_proc, p->task_ID);

		}

		break;

	case LOAN_PROCESSOR_DELIVERY:

		reclustering.pending_loan_delivery--;

#if RECLUSTERING_DEBUG
		puts("\nReceive LOAN_PROCESSOR_DELIVERY "); puts(" from proc "); puts(itoh(p->source_PE)); puts("\n");
		putsv("Numeros de delivery pendendtes: ", reclustering.pending_loan_delivery);
#endif

		if (p->allocated_processor != -1){

			if (p->hops < reclustering.min_loan_proc_hops){

#if RECLUSTERING_DEBUG
				puts("Alocou processador "); puts(itoh(p->allocated_processor)); puts("\n");
#endif

				reclustering.min_loan_proc_hops = p->hops;

				if (reclustering.current_borrowed_proc != -1){

					send_loan_proc_release(reclustering.current_borrowed_master, reclustering.current_borrowed_proc, p->task_ID);
				}

				reclustering.current_borrowed_proc = p->allocated_processor;

				reclustering.current_borrowed_master = p->source_PE;

			} else {

				send_loan_proc_release(p->source_PE, p->allocated_processor, p->task_ID);

			}
		}

		if (reclustering.pending_loan_delivery == 0){

#if RECLUSTERING_DEBUG
			puts("Fim da rodada\n");
#endif

			//handle end of reclustering round()
			if (reclustering.current_borrowed_proc != -1){

				//At this point the reclustering has benn finished successifully
				puts("Reclustering acacabou com sucesso, tarefa alocada no proc "); puts(itoh(reclustering.current_borrowed_proc)); putsv(" com id ", reclustering.task->id);

				reclustering.active = 0;

				reclustering.task->allocated_proc = reclustering.current_borrowed_proc;

				reclustering.task->borrowed_master = reclustering.current_borrowed_master;

				//Searches for a new task waiting reclustering
				app = get_application_ptr((reclustering.task->id >> 8));

				reclustering_next_task(app);

				break;



			} else {

				reclustering.neighbors_level++;

				if (reclustering.neighbors_level > max_neighbors_level){

					reclustering.neighbors_level = 1;
					puts("Warning: reclustering repeated!\n");

				}

				fires_reclustering_protocol();
			}
		}

		break;

	case LOAN_PROCESSOR_RELEASE:

#if RECLUSTERING_DEBUG
		puts("\nReceive LOAN_PROCESSOR_RELEASE "); puts(" from proc "); puts(itoh(p->source_PE)); putsv(" task id ", p->task_ID);
#endif

		page_released(clusterID, p->released_proc, p->task_ID);

		break;
	}
}

/**Searches for first application task not mapped yet, firing the reclustering
 * \param app Application pointer
 * \return 1 - if the application have all its task mapped, 0 - if the reclustering was initiated
 */
int reclustering_next_task(Application *app){

	Task *t = 0;

	/*Select the app task waiting for reclustering*/
	for (int i=0; i<app->tasks_number; i++){

		/*If task is not allocated to any processor*/
		if (app->tasks[i].allocated_proc == -1){

			t = &app->tasks[i];

			break;
		}
	}

	/*Means that the application has all its task already mapped*/
	if (t == 0){
		return 1;
	}


#if RECLUSTERING_DEBUG
		putsv("\nReclustering next task", t->id);
#endif

	/*fires reclustering protocol*/
	reclustering.active = 1;

	reclustering.task = t;

	reclustering.neighbors_level = 1;

	fires_reclustering_protocol();

	return 0;
}
