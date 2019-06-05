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

/** Allocate resources to a Cluster by decrementing the number of free resources. If the number of resources
 * is higher than free_resources, then free_resourcers receives zero, and the remaining of resources are allocated
 * by reclustering
 * \param cluster_index Index of cluster to allocate the resources
 * \param nro_resources Number of resource to allocated. Normally is the number of task of an application
 */
void allocate_cluster_resource(int cluster_index, int nro_resources){

	puts("\n\n Cluster address "); puts(itoh(cluster_info[cluster_index].master_x << 8 | cluster_info[cluster_index].master_y)); puts(" resources "); puts(itoa(cluster_info[cluster_index].free_resources));

	if (cluster_info[cluster_index].free_resources > nro_resources){
		cluster_info[cluster_index].free_resources -= nro_resources;
	} else {
		cluster_info[cluster_index].free_resources = 0;
	}

	putsv(" ALLOCATE - nro resources : ", cluster_info[cluster_index].free_resources);
	puts("\n\n");
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




/** Maps a task into a cluster processor. This function only selects the processor not modifying any management structure
 * The mapping heuristic is based on the processor's utilization (slack time) and the number of free_pages
 * \param task_id ID of the task to be mapped
 * \return Address of the selected processor
 */
int map_task(int task_id){

	int proc_address;
	int canditate_proc = -1;
	int max_slack_time = -1;
	int slack_time;

	//putsv("Mapping call for task id ", task_id);

#if MAX_STATIC_TASKS
	//Test if the task is statically mapped
	for(int i=0; i<MAX_STATIC_TASKS; i++){

		//Test if task_id is statically mapped
		if (static_map[i][0] == task_id){
			puts("Task id "); puts(itoa(static_map[i][0])); puts(" statically mapped at processor"); puts(itoh(static_map[i][1])); puts("\n");

			proc_address = static_map[i][1];

			if (get_proc_free_pages(proc_address) <= 0){
				puts("ERROR: Processor not have free resources\n");
				while(1);
			}

			return proc_address;
		}
	}
#endif

	//Else, map the task following a CPU utilization based algorithm
	for(int i=0; i<MAX_CLUSTER_SLAVES; i++){

		proc_address = get_proc_address(i);

		if (get_proc_free_pages(proc_address) > 0){

			slack_time = get_proc_slack_time(proc_address);

			if (max_slack_time < slack_time){
				canditate_proc = proc_address;
				max_slack_time = slack_time;
			}
		}
	}

	if (canditate_proc != -1){

		puts("Task mapping for task "), puts(itoa(task_id)); puts(" maped at proc "); puts(itoh(canditate_proc)); puts("\n");

		return canditate_proc;
	}

	putsv("WARNING: no resources available in cluster to map task ", task_id);
	return -1;
}

/**This heuristic maps all task of an application
 * Note that some task can not be mapped due the cluster is full or the processors not satisfies the
 * task requiriments. In this case, the reclustering will be used after the application_mapping funcion calling
 * into the kernel_master.c source file
 * Clearly the tasks that need reclustering are those one that have he allocated_processor equal to -1
 * \param cluster_id ID of the cluster where the application will be mapped
 * \param app_id ID of the application to be mapped
 * \return 1 if mapping OK, 0 if there are no available resources
*/
int application_mapping(int cluster_id, int app_id){

	Application * app;
	Task *t;
	int proc_address;

	app = get_application_ptr(app_id);

	//puts("\napplication_mapping\n");

	for(int i=0; i<app->tasks_number; i++){

		t = &app->tasks[i];

		//putsv("Vai mapear task id: ", t->id);

		//Search for static mapping
		if (t->allocated_proc != -1){
			proc_address = t->allocated_proc;
			puts("Task id "); puts(itoa(t->id)); puts(" statically mapped at processor "); puts(itoh(proc_address)); puts("\n");
		} else
			//Calls task mapping algorithm
			proc_address = map_task(t->id);


		if (proc_address == -1){

			return 0;

		} else {

			t->allocated_proc = proc_address;

			page_used(cluster_id, proc_address, t->id);

		}
	}

	puts("App have all its task mapped\n");

	return 1;
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

