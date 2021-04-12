#include <stdbool.h>
#include <stddef.h>

#include <stdlib.h>

#include "mapper.h"
#include "tag.h"

void map_init(mapper_t *mapper)
{
	mapper->available_slots = PKG_MAX_LOCAL_TASKS*PKG_N_PE;
	mapper->pending_task_cnt = 0;
	mapper->fail_map_cnt = 0;
	mapper->appid_cnt = 0;
	mapper->pending_map_app = NULL;

	task_init(mapper->tasks);
	app_init(mapper->apps);
	processor_init(mapper->processors);
}

void map_new_app(mapper_t *mapper, unsigned task_cnt, int *descriptor, unsigned desc_sz)
{
	Echo("New app received.\n");
	// Echo("Descriptor size: "); Echo(itoa(desc_sz)); Echo("\n");
	Echo("App ID: "); Echo(itoa(mapper->appid_cnt)); Echo("\n");
	Echo("Task cnt: "); Echo(itoa(task_cnt)); Echo("\n");

	if(task_cnt > mapper->available_slots){
		Echo("No available slots.\n");

		/* Save pending app descriptor and try to map on TASK_RELEASE */
		mapper->pending_task_cnt = task_cnt;
		mapper->pending_descr_sz = desc_sz;
		/** @todo memcpy */
		for(int i = 0; i < desc_sz; i++)
			mapper->pending_descr[i] = descriptor[i];
	} else {
		map_try_mapping(mapper, mapper->appid_cnt, descriptor, task_cnt, mapper->processors);
	}
}

app_t *map_build_app(mapper_t *mapper, int appid, int *descriptor, unsigned task_cnt)
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
		int task_id = descriptor[i*TASK_DESCRIPTOR_SIZE];
		app->task[task_id]->status = BLOCKED;
		// Echo("Task ID: "); Echo(itoa(task_id)); Echo("\n");
		app->task[task_id] = task_get_free(mapper->tasks);

		app->task[task_id]->id = appid << 8 | task_id;

		int proc_idx = descriptor[i*TASK_DESCRIPTOR_SIZE + 1];
		// Echo("Processor address: "); Echo(itoa(proc_idx));
		if(proc_idx != -1)
			proc_idx = (proc_idx >> 8)*PKG_N_PE_X + (proc_idx & 0xFF);
		// Echo("Processor index: "); Echo(itoa(proc_idx)); Echo("\n");
		app->task[task_id]->proc_idx = proc_idx;
		// Echo("Processor index address: "); Echo(itoa(mapper->processors[proc_idx].addr)); Echo("\n");

		app->task[task_id]->type_tag = descriptor[i*TASK_DESCRIPTOR_SIZE + 2];
		// Echo("Task type tag: "); Echo(itoa(app->task[task_id]->type_tag)); Echo("\n");

		app->task[task_id]->code_sz = descriptor[i*TASK_DESCRIPTOR_SIZE + 3];
		app->task[task_id]->data_sz = descriptor[i*TASK_DESCRIPTOR_SIZE + 4];
		app->task[task_id]->bss_sz = descriptor[i*TASK_DESCRIPTOR_SIZE + 5];
		app->task[task_id]->init_addr = descriptor[i*TASK_DESCRIPTOR_SIZE + 6];
	}

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
				Echo("No available page for statically mapped task "); Echo(itoa(app->task[i]->id)); Echo("\n");
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
				Echo("Statically mapped task "); Echo(itoa(app->task[i]->id)); Echo("at address "); Echo(itoa(processors[proc_idx].addr));
				processors[proc_idx].pending_map_cnt = 0;
				processors[proc_idx].free_page_cnt--;
			}
		}

		/* Now map dynamic tasks */
		for(int i = 0; i < app->task_cnt; i++){
			if(app->task[i]->proc_idx == -1){
				int proc_idx = processors_get_first_most_free(processors, -1);
				Echo("Dinamically mapped task "); Echo(itoa(app->task[i]->id)); Echo("at address "); Echo(itoa(processors[proc_idx].addr));
				processors[proc_idx].free_page_cnt--;
				app->task[i]->proc_idx = proc_idx;
			}
		}

		return 0;
	}
}

void map_task_allocated(mapper_t *mapper, int id)
{
	Echo("Received task allocated from id "); Echo(itoa(id)); Echo("\n");

	int appid = id >> 8;

	app_t *app = app_search(mapper->apps, appid);
	app->allocated_cnt++;

	if(app->allocated_cnt == app->task_cnt){
		/* All tasks allocated, send task release */
		Echo("All tasks allocated. Sending TASK_RELEASE\n");

		map_task_release(mapper, app);
		map_app_mapping_complete(app);
		mapper->appid_cnt++;
	}
}

void map_task_release(mapper_t *mapper, app_t *app)
{
	/* Assemble and send task release */
	Message msg;
	msg.msg[0] = TASK_RELEASE;
	// msg.msg[1] = appid_shift | i;
	// msg.msg[2] = app->task[i]->data_sz;
	// msg.msg[3] = app->task[i]->bss_sz;
	// msg.msg[4] = observer_task;
	// msg.msg[5] = observer_address;
	msg.msg[6] = app->task_cnt;

	for(int i = 0; i < app->task_cnt; i++)
		msg.msg[i + 7] = mapper->processors[app->task[i]->proc_idx].addr;
	
	msg.length = app->task_cnt + 7;

	int appid_shift = app->id << 8;
	for(int i = 0; i < app->task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		msg.msg[1] = appid_shift | i;
		msg.msg[2] = app->task[i]->data_sz;
		msg.msg[3] = app->task[i]->bss_sz;

		task_t *observer = map_nearest_tag(mapper, &(mapper->apps[0]), msg.msg[i + 7], (OBSERVE | O_QOS));

		if(observer == NULL || app->id == 0){
			msg.msg[4] = -1;
			msg.msg[5] = -1;
		} else {
			msg.msg[4] = observer->id;
			msg.msg[5] = mapper->processors[observer->proc_idx].addr;

			// Echo("Picked observer id: "); Echo(itoa(observer->id)); Echo(" at "); Echo(itoa(mapper->processors[observer->proc_idx].addr));
		}

		/* Send message directed to kernel at task address */
		SSend(&msg, KERNEL_MSG | msg.msg[i + 7]);

		/* Mark task as running */
		app->task[i]->status = RUNNING;
	}
}

void map_app_mapping_complete(app_t *app)
{
	Message msg;
	msg.msg[0] = APP_MAPPING_COMPLETE;
	msg.length = 1;
	if(app->id == 0){
		SSend(&msg, MAINJECTOR);

		msg.msg[0] = RELEASE_PERIPHERAL;
		msg.length = 1;
		SSend(&msg, (APP_INJECTOR & ~0xE0000000) | KERNEL_MSG);
	} else {
		SSend(&msg, APP_INJECTOR);
	}
}

void map_task_terminated(mapper_t *mapper, int id)
{
	Echo("Received task terminated from id "); Echo(itoa(id)); Echo("\n");

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
		Echo("App terminated. ID: "); Echo(itoa(app->id)); Echo("; At time: "); Echo(itoa(GetTick())); Echo("\n");
		app->id = -1;
	}

	if(mapper->pending_task_cnt > 0 && mapper->available_slots >= mapper->pending_task_cnt){
		/* Pending NEW_APP and resources freed. Map pending task */
		map_try_mapping(mapper, mapper->appid_cnt, mapper->pending_descr, mapper->pending_task_cnt, mapper->processors);
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
	Echo("Mapping success! Requesting task allocation.\n");

	/* Ask injector for task allocation */
	Message msg;

	msg.msg[0] = APP_ALLOCATION_REQUEST;
	int *payload = &msg.msg[1];

	for(int i = 0; i < app->task_cnt; i++){
		payload[i*2] = app->task[i]->id;
		payload[i*2 + 1] = processors[app->task[i]->proc_idx].addr;
	}

	msg.length = app->task_cnt * 2 + 1;

	if(app->id == 0)
		SSend(&msg, MAINJECTOR);
	else
		SSend(&msg, APP_INJECTOR);
}

void map_try_mapping(mapper_t *mapper, int appid, int *descr, int task_cnt, processor_t *processors)
{
	Echo("Mapping application...\n");
		
	app_t *app = map_build_app(mapper, mapper->appid_cnt, descr, task_cnt);

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
	
	Message msg;
	msg.msg[0] = SERVICE_PROVIDER;
	msg.msg[1] = tag;
	msg.msg[2] = id;
	msg.length = 3;
	SSend(&msg, requester);
}
