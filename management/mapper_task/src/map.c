#include <stdbool.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdio.h>
#include <memphis.h>

#include "mapper.h"
#include "sliding_window.h"
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
	printf("New app received at %d\n", memphis_get_tick());
	// printf("App ID: %d\n", mapper->appid_cnt);
	// printf("Task cnt: %d\n", task_cnt);

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
		app->task[i] = task_get_free(mapper->tasks);

		app->task[i]->id = appid << 8 | i;
		// Echo("Task ID: "); Echo(itoa(task_id)); Echo("\n");

		int proc_idx = descriptor[i*TASK_DESCRIPTOR_SIZE];
		// Echo("Processor address: "); Echo(itoa(proc_idx));
		if(proc_idx != -1)
			proc_idx = (proc_idx >> 8) + (proc_idx & 0xFF)*PKG_N_PE_X;
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

unsigned map_try_static(app_t *app, processor_t *processors)
{
	unsigned fail_cnt = 0;
	app->has_static_tasks = false;

	/* First check for static mapping availability */
	for(int i = 0; i < app->task_cnt; i++){
		int proc_idx = app->task[i]->proc_idx;
		if(proc_idx != -1){
			/* Statically mapped task found */
			app->has_static_tasks = true;

			/* This is needed because more than 1 task can be statically mapped to the same processor */
			processors[proc_idx].pending_map_cnt++;
			if(processors[proc_idx].pending_map_cnt > processors[proc_idx].free_page_cnt){
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
		int proc_idx = app->task[i]->proc_idx;
		if(proc_idx != -1){
			// printf("Statically mapped task %d at address %x\n", app->task[i]->id, processors[proc_idx].addr);
			// processors[proc_idx].pending_map_cnt = 0;
			processors[proc_idx].free_page_cnt--;
			app->center_x += processors[proc_idx].addr >> 8;
			app->center_y += processors[proc_idx].addr & 0xFF;
			static_cnt++;
		}
	}

	/* Will not divide by 0. Function only called on has_static_tasks == true */
	app->center_x /= static_cnt;
	app->center_y /= static_cnt;
}

void map_select_window(app_t *app, processor_t *processors, window_t *window)
{
	bool raise_x = false;
	int wx = MAP_MIN_WX;
	int wy = MAP_MIN_WY;

	while(wx*wy*PKG_MAX_LOCAL_TASKS < app->task_cnt){
		if(raise_x){
			wx++;
			raise_x = false;
		} else {
			wy++;
			raise_x = true;
		}
	}

	if(wx > PKG_N_PE_X)
		wx = PKG_N_PE_X;

	if(wy > PKG_N_PE/PKG_N_PE_X)
		wy = PKG_N_PE/PKG_N_PE_X;

	if(app->has_static_tasks){
		/* Select a window without changing last_window */
		window_set_from_center(processors, app, window, app->task_cnt, wx, wy, raise_x);
		
	} else {
		static int last_x = PKG_N_PE_X - MAP_MIN_WX;
		static int last_y = PKG_N_PE/PKG_N_PE_X - MAP_MIN_WY;

		if(wx != MAP_MIN_WX || wy != MAP_MIN_WY){
			last_x = PKG_N_PE_X - wx;
			last_y = PKG_N_PE/PKG_N_PE_X - wy;
		}
		// printf("Starting window size is %dx%d\n", last_window.wx, last_window.wy);

		window->x = last_x;
		window->y = last_y;
		window->wx = wx;
		window->wy = wy;
		map_next_window(window);

		while(true){
			/* From last window to top right corner */
			while(window->x > last_x || window->y > last_y){

				if(window_has_pages(processors, window, app->task_cnt)){
					last_x = window->x;
					last_y = window->y;
					return;
				}

				map_next_window(window);
			}

			/* From bottom left corner to last window */
			while(window->x < last_x || window->y < last_y){

				if(window_has_pages(processors, window, app->task_cnt)){
					last_x = window->x;
					last_y = window->y;
					return;
				}

				map_next_window(window);
			}

			/* Exactly last window */
			if(window_has_pages(processors, window, app->task_cnt)){
				last_x = window->x;
				last_y = window->y;
				return;
			}

			/* No window found */
			if(raise_x){
				window->wx++;
				raise_x = false;
			} else {
				window->wy++;
				raise_x = true;
			}
			last_x = PKG_N_PE_X - wx;
			last_y = PKG_N_PE/PKG_N_PE_X - wy;

			window->x = last_x;
			window->y = last_y;
			map_next_window(window);
		}	
	}
}

void map_next_window(window_t *window)
{
	if(window->x + window->wx < PKG_N_PE_X){
		window->x += MAP_STRIDE;

		if(window->x + window->wx > PKG_N_PE_X)
			window->x = PKG_N_PE_X - window->wx;
	} else if(window->y + window->wy < PKG_N_PE/PKG_N_PE_X){
		window->y += MAP_STRIDE;
		window->x = 0;

		if(window->y + window->wy > PKG_N_PE/PKG_N_PE_X)
			window->y = PKG_N_PE/PKG_N_PE_X - window->wy;
	} else {
		window->x = 0;
		window->y = 0;
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
		printf("Sending TASK_RELEASE at time %d for app %d\n", memphis_get_tick(), app->id);

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
	// msg.payload[2] = observer_task;
	// msg.payload[3] = observer_address;
	msg.payload[4] = app->task_cnt;

	for(int i = 0; i < app->task_cnt; i++)
		msg.payload[i + 5] = mapper->processors[app->task[i]->proc_idx].addr;
	
	msg.length = app->task_cnt + 5;

	int appid_shift = app->id << 8;
	for(int i = 0; i < app->task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		msg.payload[1] = appid_shift | i;

		task_t *observer = map_nearest_tag(mapper, &(mapper->apps[0]), msg.payload[i + 5], (OBSERVE | O_QOS));

		if(observer == NULL || app->id == 0){
			msg.payload[2] = -1;
			msg.payload[3] = -1;
		} else {
			msg.payload[2] = observer->id;
			msg.payload[3] = mapper->processors[observer->proc_idx].addr;

			// Echo("Picked observer id: "); Echo(itoa(observer->id)); Echo(" at "); Echo(itoa(mapper->processors[observer->proc_idx].addr));
		}

		/* Send message directed to kernel at task address */
		memphis_send_any(&msg, MEMPHIS_KERNEL_MSG | msg.payload[i + 5]);

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

	for(int i = 0; i < app->task_cnt - 1 && app->task[taskid]->consumers[i] != NULL; i++)
		app->task[taskid]->consumers[i] = NULL;
	
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
		/* Pending NEW_APP and resources freed. Map pending application which isn't built yet */
		map_try_mapping(mapper, mapper->appid_cnt, mapper->pending_task_cnt, mapper->pending_descr, mapper->pending_comm, mapper->processors);
		mapper->pending_task_cnt = 0;
	} else if(
		mapper->processors[proc_idx].failed_map &&													/* Pending static map at desired PE */
		mapper->processors[proc_idx].free_page_cnt >= mapper->processors[proc_idx].pending_map_cnt	/* All slots needed in this processor are freed! */
	){
		mapper->processors[proc_idx].failed_map = false;
		mapper->fail_map_cnt--;
		if(mapper->fail_map_cnt == 0){
			/* All needed processor slots are freed. Map and allocate now */
			map_static_tasks(mapper->pending_map_app, mapper->processors);

			map_sliding_window(mapper->pending_map_app, mapper->processors);

			/* Send task allocation to injector */
			map_task_allocation(mapper->pending_map_app, mapper->processors);

			map_set_score(mapper->pending_map_app, mapper->processors);

			mapper->pending_map_app = NULL;

			mapper->available_slots -= app->task_cnt;
		}
	}
}

void map_set_score(app_t *app, processor_t *processors)
{
	unsigned edges = 0;
	unsigned cost = 0;
	for(int i = 0; i < app->task_cnt; i++){
		task_t *producer = app->task[i];
		processors[producer->proc_idx].pending_map_cnt = 0;
		for(int j = 0; j < app->task_cnt - 1 && producer->consumers[j] != NULL; j++){
			task_t *consumer = producer->consumers[j];
			cost += map_manhattan_distance(processors[producer->proc_idx].addr, processors[consumer->proc_idx].addr);
			edges++;
		}
	}
	unsigned score = edges ? cost * 100 / edges : 0; /* Careful with division by zero */
	printf("Mapped with score %u\n", score);
	app->mapping_score = score;
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

	/**
	 * Separate into 2 phases:
	 * 1st phase: static mapping
	 * 	If if fails, keep the fail count and failed PEs flags to wait for TASK_TERMINATED
	 *  @todo
	 * 		Return a window center to change the LAST_WINDOW, to keep the window near the tasks
	 */
	mapper->fail_map_cnt = map_try_static(app, processors);

	if(!mapper->fail_map_cnt){
		/* 2nd phase: dynamic mapping (guaranteed to suceed) */
		if(app->has_static_tasks)
			map_static_tasks(app, processors);

		if(mapper->appid_cnt != 0)
			map_sliding_window(app, processors);

		/* Send task allocation to injector */
		map_task_allocation(app, processors);

		map_set_score(app, processors);
		
		/* Mapper task is already allocated */
		if(mapper->appid_cnt == 0)
			app->allocated_cnt++;
		
		mapper->available_slots -= app->task_cnt;

	} else {
		mapper->pending_map_app = app;
	}
}

void map_get_order(app_t *app, task_t *order[])
{
	task_t *initials[app->task_cnt];
	int initial_idx = 0;

	/* For all tasks */
	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = app->task[i];

		/* Check if task has producers */
		bool producer_found = false;
		for(int j = 0; i < app->task_cnt; j++){
			if(i == j)		/* Don't search own task */
				continue;

			/* Check consumers of producer if task is present */
			task_t *producer = app->task[j];
			for(int k = 0; k < app->task_cnt - 1 && producer->consumers[k] != NULL; k++){
				task_t *consumer = producer->consumers[k];
				if(consumer == task){
					/* Task is consumer, thus not initial */
					producer_found = true;
					break;
				}
			}
			if(producer_found) /* No need to scan for other producers */
				break;
		}
		if(!producer_found){
			/* Task has no producers (is no consumer), thus is initial */
			initials[initial_idx++] = task;
		}
	}

	unsigned ordered = 0;
	unsigned order_idx = 0;

	/* The mapping order starts in each initial task */
	for(int i = 0; i < initial_idx; i++){
		order[order_idx++] = initials[i];
	
		/* Map all immediate consumers of the initial task and keep mapping its sucessors */
		map_order_consumers(order, &ordered, &order_idx, app->task_cnt);
	}

	/* One or more cyclic dependences */
	if(order_idx < app->task_cnt){
		for(int i = 0; i < app->task_cnt; i++){
			if(!map_is_task_ordered(order, app->task[i], order_idx)){
				order[order_idx++] = app->task[i];
				map_order_consumers(order, &ordered, &order_idx, app->task_cnt);
			}
		}
	}
}

void map_order_consumers(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt)
{
	while(ordered < order_idx){
		task_t *producer = order[*ordered];
		for(int i = 0; i < task_cnt - 1 && producer->consumers[i] != NULL; i++){
			/* Check if consumer is not ordered yet */
			task_t *consumer = producer->consumers[i];
			if(!map_is_task_ordered(order, consumer, *order_idx))
				order[(*order_idx)++] = consumer;
		}
		(*ordered)++;
	}
}

bool map_is_task_ordered(task_t *order[], task_t *task, unsigned order_cnt)
{
	bool task_ordered = false;
	for(int i = 0; i < order_cnt; i++){
		if(order[i] == task){
			task_ordered = true;
			break;
		}
	}
	return task_ordered;
}

void map_sliding_window(app_t *app, processor_t *processors)
{
	/* 1st step: select a window */
	window_t window;
	map_select_window(app, processors, &window);
	// printf("Selected window %dx%d\n", window.x, window.y);

	/* 2nd step: get the mapping order */
	task_t *mapping_order[app->task_cnt];
	map_get_order(app, mapping_order);

	/* 3rd step: map with the least communication cost and most parallelism */
	map_dynamic_tasks(app, mapping_order, processors, &window);
}

void map_dynamic_tasks(app_t *app, task_t *order[], processor_t *processors, window_t *window)
{
	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = order[i];

		if(task->proc_idx != -1)	/* Skip tasks already mapped */
			continue;

		int seq = sw_map_task(app, task, processors, window);

		task->proc_idx = seq;
		processors[seq].free_page_cnt--;
		processors[seq].pending_map_cnt++;
		// printf("Dinamically mapped task %d at address %x\n", task->id, processors[seq].addr);
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
