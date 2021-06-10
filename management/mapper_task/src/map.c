#include <stdbool.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdio.h>
#include <memphis.h>

#include "mapper.h"
#include "services.h"
#include "tag.h"

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
	unsigned time = memphis_get_tick();
	printf("New app received at %d\n", time);
	printf("App ID: %d\n", mapper->appid_cnt);
	printf("Task cnt: %d\n", task_cnt);

	if(task_cnt > mapper->available_slots){
		puts("No available slots.\n");

		/* Save pending app descriptor and try to map on TASK_RELEASE */
		mapper->pending_task_cnt = task_cnt;
		
		/** @todo memcpy */
		for(int i = 0; i < task_cnt * 2; i++)
			mapper->pending_descr[i] = descriptor[i];

		int comm_i = 0;
		/* Copy the communication dependence list */
		for(int i = 0; i < task_cnt; i++){
			/* For all tasks, keep copying until signaled for next task */
			int consumer;
			do {
				consumer = communication[comm_i];
				mapper->pending_comm[comm_i] = communication[comm_i];
				comm_i++;
			} while(consumer > 0);
		}
	} else {
		map_try_mapping(mapper, mapper->appid_cnt, task_cnt, descriptor, communication, mapper->processors);
	}
}

app_t *map_build_app(mapper_t *mapper, int appid, unsigned task_cnt, int *descriptor, int *communication)
{
	app_t *app = app_get_free(mapper->apps);

	app->id = appid;
	app->task_cnt = task_cnt;
	app->allocated_cnt = 0;

	for(int i = 0; i < app->task_cnt; i++){
		/**
		 * @todo Check this bug
		 * This is usually = descriptor[i*6] & 0xFF.
		 * But using & in this statement acts as | besides the proper instruction being generated and executed.
		 */
		app->task[i] = task_get_free(mapper->tasks);

		app->task[i]->id = appid << 8 | i;
		// Echo("Task ID: "); Echo(itoa(task_id)); Echo("\n");

		int proc_idx = descriptor[i*TASK_DESCRIPTOR_SIZE];
		// Echo("Processor address: "); Echo(itoa(proc_idx));
		if(proc_idx != -1)
			proc_idx = (proc_idx >> 8)*PKG_N_PE_X + (proc_idx & 0xFF);
		// Echo("Processor index: "); Echo(itoa(proc_idx)); Echo("\n");
		app->task[i]->proc_idx = proc_idx;
		// Echo("Processor index address: "); Echo(itoa(mapper->processors[proc_idx].addr)); Echo("\n");

		app->task[i]->old_proc = -1;

		app->task[i]->type_tag = descriptor[i*TASK_DESCRIPTOR_SIZE + 1];
		// Echo("Task type tag: "); Echo(itoa(app->task[task_id]->type_tag)); Echo("\n");

		app->task[i]->status = BLOCKED;
	}

	int comm_i = 0;
	for(int i = 0; i < app->task_cnt; i++){
		int cons_i = 0;
		int encoded_consumer;
		do {
			encoded_consumer = communication[comm_i++];
			int consumer = abs(encoded_consumer) - 1;

			if(consumer >= 0)
				app->task[i]->consumers[cons_i++] = app->task[consumer];

		} while(encoded_consumer > 0);
	}

	// for(int i = 0; i < app->task_cnt; i++){
	// 	printf("Task %d consumers: ", i);
	// 	for(int j = 0; j < PKG_MAX_TASKS_APP && app->task[i]->consumers[j] != NULL; j++){
	// 		printf("%d ", app->task[i]->consumers[j]->id);
	// 	}
	// }

	return app;
}

bool map_app_mapping(app_t *app, processor_t *processors)
{
	bool map_failed = false;

	/* First check for static mapping availability */
	for(int i = 0; i < app->task_cnt; i++){
		int proc_idx = app->task[i]->proc_idx;
		if(proc_idx != -1){
			/* Statically mapped task found */
			/* This is needed because more than 1 task can be statically mapped to the same processor */
			processors[proc_idx].pending_map_cnt++;
			if(processors[proc_idx].pending_map_cnt > processors[proc_idx].free_page_cnt){
				printf("No available pages for statically mapped task %d\n", app->task[i]->id);
				map_failed = true;
			}
		}
	}

	if(map_failed){
		unsigned fail_cnt = 0;
		for(int i = 0; i < PKG_N_PE; i++){
			if(processors[i].pending_map_cnt > processors[i].free_page_cnt)
				fail_cnt++;
		}
		/* Return the number of processors that need TASK_TERMINATED to retry map */
		return fail_cnt;
	} else {
		/* First map statically mapped tasks */
		for(int i = 0; i < app->task_cnt; i++){
			int proc_idx = app->task[i]->proc_idx;
			if(proc_idx != -1){
				printf("Statically mapped task %d at address %x\n", app->task[i]->id, processors[proc_idx].addr);
				processors[proc_idx].pending_map_cnt = 0;
				processors[proc_idx].free_page_cnt--;
			}
		}

		/* Now map dynamic tasks */
		for(int i = 0; i < app->task_cnt; i++){
			if(app->task[i]->proc_idx == -1){
				int proc_idx = processors_get_first_most_free(processors, -1);
				printf("Dinamically mapped task %d at address %x\n", app->task[i]->id, processors[proc_idx].addr);
				processors[proc_idx].free_page_cnt--;
				app->task[i]->proc_idx = proc_idx;
			}
		}

		return 0;
	}
}

void map_task_allocated(mapper_t *mapper, int id)
{
	printf("Received task allocated from id %d\n", id);

	int appid = id >> 8;

	app_t *app = app_search(mapper->apps, appid);
	app->allocated_cnt++;

	if(app->allocated_cnt == app->task_cnt){
		/* All tasks allocated, send task release */
		printf("All tasks allocated from app %d\n", app->id);
		printf("Sending TASK_RELEASE at time %d\n", memphis_get_tick());

		map_task_release(mapper, app);
		map_app_mapping_complete(app);
		mapper->appid_cnt++;
	}
}

void map_task_release(mapper_t *mapper, app_t *app)
{
	/* Assemble and send task release */
	message_t msg;
	msg.payload[0] = TASK_RELEASE;
	// msg.payload[1] = appid_shift | i;
	// msg.payload[2] = app->task[i]->data_sz;
	// msg.payload[3] = app->task[i]->bss_sz;
	// msg.payload[4] = observer_task;
	// msg.payload[5] = observer_address;
	msg.payload[6] = app->task_cnt;

	for(int i = 0; i < app->task_cnt; i++)
		msg.payload[i + 7] = mapper->processors[app->task[i]->proc_idx].addr;
	
	msg.length = app->task_cnt + 7;

	int appid_shift = app->id << 8;
	for(int i = 0; i < app->task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		msg.payload[1] = appid_shift | i;

		task_t *observer = map_nearest_tag(mapper, &(mapper->apps[0]), msg.payload[i + 7], (OBSERVE | O_QOS));

		if(observer == NULL || app->id == 0){
			msg.payload[4] = -1;
			msg.payload[5] = -1;
		} else {
			msg.payload[4] = observer->id;
			msg.payload[5] = mapper->processors[observer->proc_idx].addr;

			// Echo("Picked observer id: "); Echo(itoa(observer->id)); Echo(" at "); Echo(itoa(mapper->processors[observer->proc_idx].addr));
		}

		/* Send message directed to kernel at task address */
		memphis_send_any(&msg, MEMPHIS_KERNEL_MSG | msg.payload[i + 7]);

		/* Mark task as running */
		app->task[i]->status = RUNNING;
	}
}

void map_app_mapping_complete(app_t *app)
{
	message_t msg;
	msg.payload[0] = APP_MAPPING_COMPLETE;
	msg.length = 1;
	if(app->id == 0){
		memphis_send_any(&msg, MAINJECTOR);

		msg.payload[0] = RELEASE_PERIPHERAL;
		msg.length = 1;
		memphis_send_any(&msg, (APP_INJECTOR & ~0xE0000000) | MEMPHIS_KERNEL_MSG);
	} else {
		memphis_send_any(&msg, APP_INJECTOR);
	}
}

void map_task_terminated(mapper_t *mapper, int id)
{
	printf("Received task terminated from id %d at time %d\n", id, memphis_get_tick());

	int appid = id >> 8;
	int taskid = id & 0xFF;

	app_t *app = app_search(mapper->apps, appid);
	int proc_idx = app->task[taskid]->proc_idx;

	/* Deallocate */
	if(app->task[taskid]->status == MIGRATING){
		/* The task finished with a migration request on the fly */
		mapper->available_slots++;
		int old_proc = app->task[taskid]->old_proc;
		mapper->processors[old_proc].free_page_cnt++;
	}

	app->task[taskid]->id = -1;
	app->task[taskid] = NULL;
	app->allocated_cnt--;
	mapper->available_slots++;
	mapper->processors[proc_idx].free_page_cnt++;

	/* All tasks terminated, terminate app */
	if(app->allocated_cnt == 0){
		printf("App %d terminated at time %d\n", app->id, memphis_get_tick());
		app->id = -1;
	}

	if(mapper->pending_task_cnt > 0 && mapper->available_slots >= mapper->pending_task_cnt){
		/* Pending NEW_APP and resources freed. Map pending task */
		map_try_mapping(mapper, mapper->appid_cnt, mapper->pending_task_cnt, mapper->pending_descr, mapper->pending_comm, mapper->processors);
	} else if(
		mapper->fail_map_cnt > 0 &&																	/* Pending mapping */
		mapper->processors[proc_idx].pending_map_cnt > 0 && 										/* Terminated task freed desired processor */
		mapper->processors[proc_idx].free_page_cnt >= mapper->processors[proc_idx].pending_map_cnt	/* All slots needed in this processor are freed! */
	){
		mapper->processors[proc_idx].pending_map_cnt = 0;
		mapper->fail_map_cnt--;
		if(mapper->fail_map_cnt == 0){
			/* All needed processor slots are freed. Map and allocate now */
			mapper->fail_map_cnt = map_app_mapping(mapper->pending_map_app, mapper->processors);
			if(!mapper->fail_map_cnt){
				/* Send task allocation to injector */
				map_task_allocation(mapper->pending_map_app, mapper->processors);

				mapper->pending_map_app = NULL;
			}
		}
	}
}

void map_task_allocation(app_t *app, processor_t *processors)
{
	puts("Mapping success! Requesting task allocation.\n");

	/* Ask injector for task allocation */
	message_t msg;

	msg.payload[0] = APP_ALLOCATION_REQUEST;
	unsigned int *payload = &msg.payload[1];

	for(int i = 0; i < app->task_cnt; i++){
		payload[i*2] = app->task[i]->id;
		payload[i*2 + 1] = processors[app->task[i]->proc_idx].addr;
	}

	msg.length = app->task_cnt * 2 + 1;

	if(app->id == 0)
		memphis_send_any(&msg, MAINJECTOR);
	else
		memphis_send_any(&msg, APP_INJECTOR);
}

void map_try_mapping(mapper_t *mapper, int appid, int task_cnt, int *descr, int *comm, processor_t *processors)
{
	puts("Mapping application...\n");
		
	app_t *app = map_build_app(mapper, mapper->appid_cnt, task_cnt, descr, comm);

	mapper->fail_map_cnt = map_app_mapping(app, mapper->processors);
	if(!mapper->fail_map_cnt){
		/* Send task allocation to injector */
		map_task_allocation(app, processors);
		
		/* Mapper task is already allocated */
		if(mapper->appid_cnt == 0)
			app->allocated_cnt++;

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
				unsigned new_dist = map_manhattan_distance(address, mapper->processors[ma->task[i]->proc_idx].addr);
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
	
	message_t msg;
	msg.payload[0] = SERVICE_PROVIDER;
	msg.payload[1] = tag;
	msg.payload[2] = id;
	msg.length = 3;
	memphis_send_any(&msg, requester);
}
