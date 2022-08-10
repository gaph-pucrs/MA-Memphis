/**
 * MA-Memphis
 * @file mapper.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main mapper functions
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memphis.h>

#include "mapper.h"
#include "sliding_window.h"
#include "services.h"
#include "oda.h"

void map_init(mapper_t *mapper)
{
	mapper->pending_task_cnt = 0;

	mapper->pending_map_app = NULL;

	mapper->fail_map_cnt = 0;

	mapper->available_slots = PKG_MAX_LOCAL_TASKS*PKG_N_PE;
	mapper->appid_cnt = 0;

	app_init(mapper->apps);
	task_init(mapper->tasks);
	processor_init(mapper->processors);
}

void map_new_app(mapper_t *mapper, unsigned task_cnt, int *descriptor, int *communication)
{
	printf("New app received at %d\n", memphis_get_tick());
	// printf("App ID: %d\n", mapper->appid_cnt);
	// printf("Task cnt: %d\n", task_cnt);

	if(task_cnt > mapper->available_slots){
		puts("No available slots.\n");

		/* Save pending app descriptor and try to map on TASK_RELEASE */
		mapper->pending_task_cnt = task_cnt;
		
		memcpy(mapper->pending_descr, descriptor, task_cnt * 2 * sizeof(mapper->pending_descr[0]));

		int comm_i = 0;
		/* Copy the communication dependence list */
		for(int i = 0; i < task_cnt; i++){
			/* For all tasks, keep copying until signaled for next task */
			int successor;
			do {
				successor = communication[comm_i];
				mapper->pending_comm[comm_i] = communication[comm_i];
				comm_i++;
			} while(successor > 0);
		}
	} else {
		map_try_mapping(mapper, mapper->appid_cnt, task_cnt, descriptor, communication, mapper->processors);
	}
}

unsigned map_try_static(app_t *app, processor_t *processors)
{
	unsigned fail_cnt = 0;
	app->has_static_tasks = false;

	/* First check for static mapping availability */
	for(int i = 0; i < app->task_cnt; i++){
		processor_t *processor = app->task[i]->processor;
		if(processor){
			/* Statically mapped task found */
			app->has_static_tasks = true;

			/* This is needed because more than 1 task can be statically mapped to the same processor */
			processor->pending_map_cnt++;
			if(processor->pending_map_cnt > processor->free_page_cnt){
				printf("No available pages for statically mapped task %d\n", app->task[i]->id);
				if(!processors[i].failed_map){
					fail_cnt++;
					processors[i].failed_map = true;
				}
			}
		}
	}

	return fail_cnt;
}

void map_static_tasks(app_t *app, processor_t *processors)
{
	unsigned static_cnt = 0;
	app->center_x = 0;
	app->center_y = 0;

	/* First map statically mapped tasks */
	for(int i = 0; i < app->task_cnt; i++){
		processor_t *processor = app->task[i]->processor;
		if(processor){
			// printf("Statically mapped task %d at address %x\n", app->task[i]->id, processor->addr);
			processor->pending_map_cnt = 0;
			processor_add_task(processor, app->task[i]);
			app->center_x += processor->addr >> 8;
			app->center_y += processor->addr & 0xFF;
			static_cnt++;
		}
	}

	/* Will not divide by 0. Function only called on has_static_tasks == true */
	app->center_x /= static_cnt;
	app->center_y /= static_cnt;
}

void map_task_allocated(mapper_t *mapper, int id)
{
	printf("Received task allocated from id %d\n", id);

	int appid = id >> 8;

	app_t *app = app_search(mapper->apps, appid);
	app->allocated_cnt++;

	if(app->allocated_cnt == app->task_cnt){
		/* All tasks allocated, send task release */
		printf("Sending TASK_RELEASE at time %d for app %d\n", memphis_get_tick(), app->id);

		map_task_release(mapper, app);
		map_app_mapping_complete(app);
		mapper->appid_cnt++;
	}
}

void map_task_release(mapper_t *mapper, app_t *app)
{
	/* Assemble and send task release */
	size_t msg_size = (app->task_cnt + 3)*sizeof(int);
	int *out_msg = malloc(msg_size);
	out_msg[0] = TASK_RELEASE;
	// out_msg[1] = appid_shift | i;
	out_msg[2] = app->task_cnt;

	for(int i = 0; i < app->task_cnt; i++)
		out_msg[i + 3] = app->task[i]->processor->addr;

	int appid_shift = app->id << 8;
	for(int i = 0; i < app->task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		out_msg[1] = appid_shift | i;

		/* Send message directed to kernel at task address */
		memphis_send_any(out_msg, msg_size, MEMPHIS_KERNEL_MSG | out_msg[i + 3]);

		/* Mark task as running */
		app->task[i]->status = RUNNING;
	}

	free(out_msg);
	out_msg = NULL;
}

void map_app_mapping_complete(app_t *app)
{
	int out_msg = APP_MAPPING_COMPLETE;
	
	if(app->id == 0){
		memphis_send_any(&out_msg, sizeof(out_msg), MAINJECTOR);

		memphis_br_send_all(0, RELEASE_PERIPHERAL);
	} else {
		memphis_send_any(&out_msg, sizeof(out_msg), APP_INJECTOR);
	}
}

void map_task_terminated(mapper_t *mapper, int id)
{
	printf("Received task terminated from id %d at time %d\n", id, memphis_get_tick());

	int appid = id >> 8;
	int taskid = id & 0xFF;
	app_t *app = app_search(mapper->apps, appid);

	processor_t *processor = app->task[taskid]->processor;

	/* Terminate task */
	processor_t *old_proc = task_terminate(app->task[taskid]);
	if(old_proc){
		/* The task finished with a migration request on the fly */
		mapper->available_slots++;
		processor_remove_task(old_proc, app->task[taskid]);
	}

	map_dealloc(mapper, app, taskid);

	/* All tasks terminated, terminate app */
	if(app->allocated_cnt == 0){
		printf("App %d terminated at time %d\n", app->id, memphis_get_tick());
		app->id = -1;
	}

	map_test_pending(mapper, processor, old_proc);
}

void map_set_score(app_t *app, processor_t *processors)
{
	unsigned edges = 0;
	unsigned cost = 0;
	for(int i = 0; i < app->task_cnt; i++){
		task_t *predecessor = app->task[i];
		for(int j = 0; j < predecessor->succ_cnt; j++){
			task_t *successor = predecessor->successors[j];
			cost += map_manhattan_distance(predecessor->processor->addr, successor->processor->addr);
			edges++;
		}
	}
	unsigned score = edges ? cost * 100 / edges : 0; /* Careful with division by zero */
	printf("Mapped with score %u at %d\n", score, memphis_get_tick());
	app->mapping_score = score;
}

void map_task_allocation(app_t *app, processor_t *processors)
{
	// puts("Mapping success! Requesting task allocation.\n");

	/* Ask injector for task allocation */
	size_t msg_size = (app->task_cnt * 2 + 1)*sizeof(int);
	int *out_msg = malloc(msg_size);

	out_msg[0] = APP_ALLOCATION_REQUEST;
	int *payload = &out_msg[1];

	for(int i = 0; i < app->task_cnt; i++){
		payload[i*2] = app->task[i]->id;
		payload[i*2 + 1] = app->task[i]->processor->addr;
	}

	if(app->id == 0)
		memphis_send_any(out_msg, msg_size, MAINJECTOR);
	else
		memphis_send_any(out_msg, msg_size, APP_INJECTOR);

	free(out_msg);
	out_msg = NULL;
}

void map_try_mapping(mapper_t *mapper, int appid, int task_cnt, int *descr, int *comm, processor_t *processors)
{
	// puts("Mapping application...\n");
		
	app_t *app = app_get_free(mapper->apps);
	app_build(app, mapper->appid_cnt, task_cnt, descr, comm, mapper->tasks, processors);

	/* 1st phase: static mapping */
	mapper->fail_map_cnt = map_try_static(app, processors);

	if(!mapper->fail_map_cnt){
		/* 2nd phase: dynamic mapping (guaranteed to suceed) */
		if(app->has_static_tasks)
			map_static_tasks(app, processors);

		if(mapper->appid_cnt != 0)
			sw_map_app(app, processors);

		/* Send task allocation to injector */
		map_task_allocation(app, processors);

		map_set_score(app, processors);
		
		/* Mapper task is already allocated */
		if(mapper->appid_cnt == 0){
			app->allocated_cnt++;
			/* Check if mapper task is the only MA task */
			if(app->allocated_cnt == app->task_cnt){
				map_app_mapping_complete(app);
				mapper->appid_cnt++;
			}
		}
		
		mapper->available_slots -= app->task_cnt;
	} else {
		mapper->pending_map_app = app;
	}
}

task_t *map_nearest_tag(mapper_t *mapper, app_t *ma, int address, unsigned tag)
{
	task_t *oda = NULL;
	unsigned distance = -1;

	/* Search all Management tasks */
	/* Don't break if task id is -1 because MA IDs could be reused in the future */
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(ma->task[i] != NULL){
			// Echo("Trying MA id "); Echo(itoa(ma->task[i]->id)); Echo("\n");
			// Echo("Task tag is "); Echo(itoa(ma->task[i]->type_tag)); Echo("\n");

			if((ma->task[i]->type_tag & tag) == tag){
				/* Tag found! Check distance */
				unsigned new_dist = map_manhattan_distance(address, ma->task[i]->processor->addr);
				if(new_dist < distance){
					distance = new_dist;
					oda = ma->task[i];
				}
			}
		}
	}

	return oda;
}

unsigned map_manhattan_distance(int source, int target)
{
	int src_x = source >> 8;
	int src_y = source & 0xFF;

	int tgt_x = target >> 8;
	int tgt_y = target & 0xFF;

	int dist_x = abs(src_x - tgt_x);
	int dist_y = abs(src_y - tgt_y);

	return dist_x + dist_y;
}

void map_request_service(mapper_t *mapper, int address, unsigned tag, int requester)
{
	task_t *oda = map_nearest_tag(mapper, &mapper->apps[0], address, tag);

	int id = -1;
	if(oda != NULL)
		id = oda->id;
	
	int out_msg[] = {SERVICE_PROVIDER, tag, id};
	
	memphis_send_any(out_msg, sizeof(out_msg), requester);
}

void map_task_aborted(mapper_t *mapper, int id)
{
	printf("Received task aborted from id %d at time %d\n", id, memphis_get_tick());

	int appid = id >> 8;
	int taskid = id & 0xFF;
	app_t *app = app_search(mapper->apps, appid);

	processor_t *processor = app->task[taskid]->processor;

	/* Terminate task */
	processor_t *old_proc = task_terminate(app->task[taskid]);
	if(old_proc){
		/* The task finished with a migration request on the fly */
		mapper->available_slots++;
		processor_remove_task(old_proc, app->task[taskid]);
	}

	map_dealloc(mapper, app, taskid);

	/* All tasks terminated, terminate app */
	if(app->allocated_cnt == 0){
		printf("App %d aborted at time %d\n", app->id, memphis_get_tick());
		app->id = -1;
	}

	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = app->task[i];
		if(task){
			/* Task is running and needs to be aborted */
			int addr = task->status != MIGRATING ? task->processor->addr : task->old_proc->addr;
			uint32_t payload = task->id;
			memphis_br_send_tgt(payload, addr, ABORT_TASK);
		}
	}

	map_test_pending(mapper, processor, old_proc);
}

void map_dealloc(mapper_t *mapper, app_t *app, int taskid)
{	
	/* Deallocate task from app */
	app->task[taskid] = NULL;
	app->allocated_cnt--;

	/* Deallocate task from mapper */
	mapper->available_slots++;
}

void map_test_pending(mapper_t *mapper, processor_t *processor, processor_t *old_proc)
{
	if(mapper->pending_task_cnt > 0 && mapper->available_slots >= mapper->pending_task_cnt){
		/* Pending NEW_APP and resources freed. Map pending application which isn't built yet */
		map_try_mapping(mapper, mapper->appid_cnt, mapper->pending_task_cnt, mapper->pending_descr, mapper->pending_comm, mapper->processors);
		mapper->pending_task_cnt = 0;
	} else {
		map_test_failed(mapper, processor);
		if(old_proc)
			map_test_failed(mapper, old_proc);
	} 
}

void map_test_failed(mapper_t *mapper, processor_t *processor)
{
	if(
		processor->failed_map &&								/* Pending static map at desired PE */
		processor->free_page_cnt >= processor->pending_map_cnt	/* All slots needed in this processor are freed! */
	){
		processor->failed_map = false;
		mapper->fail_map_cnt--;
		if(mapper->fail_map_cnt == 0){
			/* All needed processor slots are freed. Map and allocate now */
			map_static_tasks(mapper->pending_map_app, mapper->processors);

			sw_map_app(mapper->pending_map_app, mapper->processors);

			/* Send task allocation to injector */
			map_task_allocation(mapper->pending_map_app, mapper->processors);

			map_set_score(mapper->pending_map_app, mapper->processors);

			mapper->available_slots -= mapper->pending_map_app->task_cnt;

			mapper->pending_map_app = NULL;
		}
	}
}
