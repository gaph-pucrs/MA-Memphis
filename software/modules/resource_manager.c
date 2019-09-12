/*!\file resource_manager.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief Selects where to execute a task and application
 * \detailed Cluster scheduler implements the cluster resources management,
 * task mapping, application mapping, and also can implement task migration heuristics.
 * Adittionally it have a function named: SearchCluster, which selects the cluster to send an application. This
 * function in only used in the global master mode
 */

#include "resource_manager.h"

#include "../../include/kernel_pkg.h"
#include "utils.h"
#include "processors.h"
#include "applications.h"
#include "reclustering.h"

/** Allocate resources to a Cluster by decrementing the number of free resources. If the number of resources
 * is higher than free_resources, then free_resourcers receives zero, and the remaining of resources are allocated
 * by reclustering
 * \param cluster_index Index of cluster to allocate the resources
 * \param nro_resources Number of resource to allocated. Normally is the number of task of an application
 */
void allocate_cluster_resource(int cluster_index, int nro_resources){

	//puts("\n\n Cluster address "); puts(itoh(cluster_info[cluster_index].master_x << 8 | cluster_info[cluster_index].master_y)); puts(" resources "); puts(itoa(cluster_info[cluster_index].free_resources));

	if (cluster_info[cluster_index].free_resources > nro_resources){
		cluster_info[cluster_index].free_resources -= nro_resources;
	} else {
		cluster_info[cluster_index].free_resources = 0;
	}

	//putsv(" ALLOCATE - nro resources : ", cluster_info[cluster_index].free_resources);
	//puts("\n\n");
}

/** Release resources of a Cluster by incrementing the number of free resources according to the nro of resources
 * by reclustering
 * \param cluster_index Index of cluster to allocate the resources
 * \param nro_resources Number of resource to release. Normally is the number of task of an application
 */
void release_cluster_resources(int cluster_index, int nro_resources){

	//puts(" Cluster address "); puts(itoh(cluster_info[cluster_index].master_x << 8 | cluster_info[cluster_index].master_y)); puts(" resources "); puts(itoa(cluster_info[cluster_index].free_resources));

	cluster_info[cluster_index].free_resources += nro_resources;

   // putsv(" RELEASE - nro resources : ", cluster_info[cluster_index].free_resources);
}

/** This function is called by kernel manager inside it own code and in the modules: reclustering and cluster_scheduler.
 * It is called even when a task is mapped into a processor, by normal task mapping or reclustering.
 * Automatically, this function update the Processors structure by calling the add_task function
 * \param cluster_id Index of cluster to allocate the page
 * \param proc_address Address of the processor that is receiving the task
 * \param task_ID ID of the allocated task
 */
void page_used(int cluster_id, int proc_address, int task_ID){

	//puts("Page used proc: "); puts(itoh(proc_address)); putsv(" task id ", task_ID);
	add_task(proc_address, task_ID);

	allocate_cluster_resource(cluster_id, 1);
}

/** This function is called by manager inside it own code and in the modules: reclustering and cluster_scheduler.
 * It is called even when a task is removed from a processor.
 * Automatically, this function update the Processors structure by calling the remove_task function
 * \param cluster_id Index of cluster to remove the page
 * \param proc_address Address of the processor that is removing the task
 * \param task_ID ID of the removed task
 */
void page_released(int cluster_id, int proc_address, int task_ID){

	//puts("Page released proc: "); puts(itoh(proc_address)); putsv(" task id ", task_ID);
	remove_task(proc_address, task_ID);

	release_cluster_resources(cluster_id, 1);
}


/** Searches following the diamond search paradigm. The search occurs only inside cluster
 * \param current_allocated_pe Current PE of the target task
 * \param last_proc The last proc returned by diamond_search function. 0 if is the first call
 * \return The selected processor address. -1 if not found
 */
int diamond_search_initial(int begining_core){

	int ref_x, ref_y, max_round, hop_count, max_tested_proc;//XY address of the current processor of the task
	int proc_x, proc_y, proc_count;
	int candidate_proc, test_proc;
	int max_x, max_y, min_x, min_y, master_addr;

	ref_x = (begining_core >> 8);
	ref_y = (begining_core & 0xFF);

	//Return the own beggining core if it is free
	if(get_proc_free_pages(begining_core)){
		//puts("Mapped at beggining core\n");
		return begining_core;
	}

	//puts("xi: "); puts(itoh(cluster_x_offset));

	max_x = (XCLUSTER + cluster_info[clusterID].xi);
	max_y = (YCLUSTER + cluster_info[clusterID].yi);
	min_x = cluster_info[clusterID].xi;
	min_y = cluster_info[clusterID].yi;
	master_addr = ( (cluster_info[clusterID].master_x << 8) | cluster_info[clusterID].master_y);

	max_round = 0;
	hop_count = 0;
	proc_count = 1;

	max_tested_proc = MAX_CLUSTER_SLAVES;

	//Search candidate algorithm
	while(proc_count < max_tested_proc){

		//max_round count the number of processor of each round
		max_round+=4;
		hop_count++; //hop_count is used to position the proc_y at the top of the task's processor

		proc_x = ref_x;
		proc_y = ref_y + hop_count;

		//puts(itoa(proc_x)); putsv("x", proc_y);

		candidate_proc = -1;//Init the variable to start the looping below
		//putsv("-------- New round: ", max_round);

		//Walks for all processor of the round
		for(int r=0; r<max_round; r++){

			//puts("Testando addr: "); puts(itoa(proc_x)); puts("x"); puts(itoa(proc_y)); puts("\n");

			test_proc = proc_x << 8 | proc_y;

			//Test if the current processor is out of the system dimension
			if (test_proc != master_addr && min_x <= proc_x && max_x > proc_x && min_y <= proc_y && max_y > proc_y){ // Search all PEs

				//puts("Entrou addr: "); puts(itoa(proc_x)); puts("x"); puts(itoa(proc_y)); puts("\n");
				//Increment the number of valid processors visited
				proc_count++;

				//Tests if the processor is available, if yes, mark it as used in free_core_map and return
				if(get_proc_free_pages(test_proc)){
				//if (free_core_map[proc_x][proc_y] == 1){
					candidate_proc = test_proc;
					//puts("Proc selected: "); puts(itoh(candidate_proc)); puts("\n");
					return candidate_proc;
				}

			}

			//These if walk over the processor addresses
			if (proc_y > ref_y && proc_x <= ref_x){ //up to left
					proc_y--;
					proc_x--;
					//puts("up left\n");
			} else if (proc_y <= ref_y && proc_x < ref_x){//left to bottom
					proc_y--;
					proc_x++;
					//puts("left to bottom\n");
			} else if (proc_x >= ref_x && proc_y < ref_y){//bottom to right
					proc_y++;
					proc_x++;
					//puts("bottom to right\n");
			} else {//right to up
					proc_y++;
					proc_x--;
					//puts("right to up\n");
			}
		}

	}

	return -1;
}


int application_mapping(int app_id){

	Application * app;
	Task *t;
	int proc_address;
	int initial_pe_list[MAX_CLUSTER_APP];
	int initial_size;
	int initial_app_pe;
	int proc_addr;
	int max_avg_manhatam;
	int man_sum, man_count, man_curr;
	int xi, yi, xj, yj;
	app = get_application_ptr(app_id);
	
	//Define the mapping order, this is only used to allow static map first
	int mapping_order[app->tasks_number];
	int mapping_order_count = 0;
	for(int i=0; i<app->tasks_number; i++){
		mapping_order[i] = -1;
	}

	//puts("\napplication_mapping\n");

	max_avg_manhatam = -1;
	initial_app_pe = -1;

	//puts("\n----------------Defining list of initial PE------------\n");
	get_initial_pe_list(initial_pe_list, &initial_size);

	if (initial_size){

		//For each free PE of the cluster
		for(int i=0; i<MAX_CLUSTER_SLAVES; i++){

			proc_addr = get_proc_address(i);

			if(get_proc_free_pages(proc_addr) > 0){

				man_sum = 0;
				man_count = 0;

				xi = proc_addr >> 8;
				yi = proc_addr & 0xFF;
				//For each initial_PE compute  the avg manhatam from the proc_addr
				for(int j=0; j<initial_size; j++){

					xj = initial_pe_list[j] >> 8;
					yj = initial_pe_list[j] & 0xFF;

					//Computes the manhatam distance
					man_curr = (abs(xi - xj) + abs(yi - yj));

					man_sum = man_sum + man_curr;
					man_count++;
				}

				//After the for computes the mean
				man_curr = (man_sum / man_count);
				//puts("Current Avg mean for PE "); puts(itoh(proc_addr)); putsv(" is ", man_curr);

				if (man_curr > max_avg_manhatam){
					max_avg_manhatam = man_curr;
					initial_app_pe = proc_addr;
					//puts("PE selected\n");
				}
			}

		}

	} else {
		initial_app_pe = cluster_info[clusterID].xi << 8 | cluster_info[clusterID].yf;
		//puts("Very first time\n"); puts(itoh(initial_app_pe)); puts("\n");
	}

	puts("Seleted initial PE at: "); puts(itoh(initial_app_pe)); puts("\n");


	/*First add to mapping_order the statically mapped tasks */
	for(int i=0; i<app->tasks_number; i++){
		t = &app->tasks[i];
		if (t->allocated_proc != -1){
			mapping_order[mapping_order_count] = i;
			mapping_order_count++;
		}
	}

	/*Secondly add to mapping_order the dynamically mapped tasks */
	for(int i=0; i<app->tasks_number; i++){
		t = &app->tasks[i];
		if (t->allocated_proc == -1){
			mapping_order[mapping_order_count] = i;
			mapping_order_count++;
		}
	}
	


	for(int i=0, task_i; i<app->tasks_number; i++){
		
		task_i = mapping_order[i];
		
		t = &app->tasks[task_i];

		//Search for static mapping
		if (t->allocated_proc != -1){
			proc_address = t->allocated_proc;
			puts("Task id "); puts(itoa(t->id)); puts(" statically mapped at processor "); puts(itoh(proc_address)); puts("\n");
		} else
			//Calls task mapping algorithm
			proc_address = diamond_search_initial(initial_app_pe);

		if (proc_address == -1){

			return 0;

		} else {

			t->allocated_proc = proc_address;

			page_used(clusterID, proc_address, t->id);

			puts("Task id "); puts(itoa(t->id)); puts(" dynamically mapped at processor "); puts(itoh(proc_address)); puts("\n");

		}
	}

	puts("App have all its task mapped\n");

	return 1;
}


/** Maps a task into a cluster processor. This function only selects the processor not modifying any management structure
 * The mapping heuristic is based on the processor's utilization (slack time) and the number of free_pages
 * \param task_id ID of the task to be mapped
 * \return Address of the selected processor
 */
int reclustering_map(int ref_proc){

	int ref_x, ref_y, curr_x, curr_y, proc_address;
	int curr_man, min_man, sel_proc;

	ref_x = ref_proc >> 8;
	ref_y = ref_proc & 0xFF;

	min_man = (XDIMENSION*YDIMENSION);
	sel_proc = -1;

	//Else, selects the pe with the minimal manhatam to the initial proc
	for(int i=0; i<MAX_CLUSTER_SLAVES; i++){

		proc_address = get_proc_address(i);

		curr_x = proc_address >> 8;
		curr_y = proc_address & 0xFF;

		if (get_proc_free_pages(proc_address) > 0){

			curr_man = (abs(ref_x - curr_x) + abs(ref_y - curr_y));

			if (curr_man < min_man){
				min_man = curr_man;
				sel_proc = proc_address;
			}
		}
	}

	return sel_proc;
}


/**Selects a cluster to insert an application
 * \param GM_cluster_id cluster ID of the global manager processor
 * \param app_task_number Number of task of requered application
 * \return > 0 if mapping OK, -1 if there is not resources available
*/
int SearchCluster(int GM_cluster_id, int app_task_number) {

	int selected_cluster = -1;
	int freest_cluster = 0;

	for (int i=0; i<CLUSTER_NUMBER; i++){

		if (i == GM_cluster_id) continue;

		if (cluster_info[i].free_resources > freest_cluster){
			selected_cluster = i;
			freest_cluster = cluster_info[i].free_resources;
		}
	}

	if (cluster_info[GM_cluster_id].free_resources > freest_cluster){
		selected_cluster = GM_cluster_id;
	}

	return selected_cluster;
}

