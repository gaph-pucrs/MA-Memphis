#include <stdbool.h>
#include <stddef.h>

#include <api.h>
#include <stdlib.h>

#include "mapper.h"
#include "task_migration.h"

int main()
{
	Echo("Mapper started at "); Echo(itoa(GetTick())); Echo("\n");

	mapper_t mapper;
	map_init(&mapper);

	while(true){
		static Message msg;
		SReceive(&msg);
		/* Check what service has been received */
		switch(msg.msg[0]){
			case NEW_APP:
				map_new_app(&mapper, msg.msg[2], &msg.msg[3], msg.msg[1] - 1);
				break;
			case TASK_ALLOCATED:
				map_task_allocated(&mapper, msg.msg[1]);
				break;
			case TASK_TERMINATED:
				map_task_terminated(&mapper, msg.msg[1]);
				break;
			case TASK_MIGRATION:
				tm_migrate(&mapper, msg.msg[1]);
				break;
			default:
				Echo("Invalid service received: "); Echo(itoa(msg.msg[0])); Echo("\n");
				break;
		}
	}
}

void map_init(mapper_t *mapper)
{
	mapper->available_slots = PKG_MAX_LOCAL_TASKS*PKG_N_PE;
	mapper->pending_task_cnt = 0;
	mapper->fail_map_cnt = 0;
	/** @todo Change to 0 when using more management tasks */
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
		int task_id = descriptor[i*6];
		// Echo("Task ID: "); Echo(itoa(task_id)); Echo("\n");
		app->task[task_id] = task_get_free(mapper->tasks);

		app->task[task_id]->id = appid << 8 | task_id;

		int proc_idx = descriptor[i*6 + 1];
		// Echo("Processor address: "); Echo(itoa(proc_idx));
		if(proc_idx != -1)
			proc_idx = (proc_idx >> 8)*PKG_N_PE_X + (proc_idx & 0xFF);
		// Echo("Processor index: "); Echo(itoa(proc_idx)); Echo("\n");
		app->task[task_id]->proc_idx = proc_idx;
		// Echo("Processor index address: "); Echo(itoa(mapper->processors[proc_idx].addr)); Echo("\n");

		app->task[task_id]->code_sz = descriptor[i*6 + 2];
		app->task[task_id]->data_sz = descriptor[i*6 + 3];
		app->task[task_id]->bss_sz = descriptor[i*6 + 4];
		app->task[task_id]->init_addr = descriptor[i*6 + 5];
	}

	return app;
}

app_t *app_get_free(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		if(apps[i].id == -1)
			return &apps[i];

	return NULL;
}

task_t *task_get_free(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		if(tasks[i].id == -1)
			return &tasks[i];

	return NULL;
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
				int proc_idx = processors_get_first_most_free(processors);
				Echo("Dinamically mapped task "); Echo(itoa(app->task[i]->id)); Echo("at address "); Echo(itoa(processors[proc_idx].addr));
				processors[proc_idx].free_page_cnt--;
				app->task[i]->proc_idx = proc_idx;
			}
		}

		return 0;
	}
}

void task_init(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		tasks[i].id = -1;
}

void app_init(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		apps[i].id = -1;
}

void processor_init(processor_t *processors)
{
	// Echo("Initializing "); Echo(itoa(PKG_N_PE)); Echo(" processors\n");
	for(int i = 0; i < PKG_N_PE; i++){
		processors[i].addr = i / PKG_N_PE_X << 8 | i % PKG_N_PE_X;
		// Echo("Addr "); Echo(itoa(processors[i].addr));
		processors[i].free_page_cnt = PKG_MAX_TASKS_APP;
		processors[i].pending_map_cnt = 0;
	}

	/* Mapper task temporarily only mapped to 0x0 */
	// processors[0].free_page_cnt--;
}

int processors_get_first_most_free(processor_t *processors)
{
	int address = 0;
	for(int i = 1; i < PKG_N_PE; i++){
		if(processors[i].free_page_cnt > processors[address].free_page_cnt)
			address = i;
	}
	return address;
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
		map_app_mapping_complete();
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
	msg.msg[4] = app->task_cnt;
	for(int i = 0; i < app->task_cnt; i++)
		msg.msg[i + 5] = mapper->processors[app->task[i]->proc_idx].addr;
	
	msg.length = app->task_cnt + 5;

	int appid_shift = app->id << 8;
	for(int i = 0; i < app->task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		msg.msg[1] = appid_shift | i;
		msg.msg[2] = app->task[i]->data_sz;
		msg.msg[3] = app->task[i]->bss_sz;

		/* Send message directed to kernel at task address */
		SSend(&msg, KERNEL_MSG | msg.msg[i + 5]);
	}
}

void map_app_mapping_complete()
{
	Message msg;
	msg.msg[0] = APP_MAPPING_COMPLETE;
	msg.length = 1;
	SSend(&msg, APP_INJECTOR);
}

app_t *app_search(app_t *apps, int appid)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(apps[i].id == appid)
			return &apps[i];
	}
	return NULL;
}

void map_task_terminated(mapper_t *mapper, int id)
{
	Echo("Received task terminated from id "); Echo(itoa(id)); Echo("\n");

	int appid = id >> 8;
	int taskid = id & 0xFF;

	app_t *app = app_search(mapper->apps, appid);
	int proc_idx = app->task[taskid]->proc_idx;

	/* Deallocate */
	app->task[taskid]->id = -1;
	app->task[taskid] = NULL;
	app->allocated_cnt--;
	mapper->available_slots++;
	mapper->processors[proc_idx].free_page_cnt++;

	/* All tasks terminated, terminate app */
	if(app->allocated_cnt == 0){
		Echo("App terminated. ID: "); Echo(itoa(app->id)); Echo("\n");
		app->id = -1;
	}

	if(mapper->pending_task_cnt > 0 && mapper->available_slots >= mapper->pending_task_cnt){
		/* Pending NEW_APP and resources freed. Map pending task */
		map_try_mapping(mapper, mapper->appid_cnt, mapper->pending_descr, mapper->pending_task_cnt, mapper->processors);
	} else if(
		mapper->fail_map_cnt > 0 &&																	/* Pending mapping */
		mapper->processors[proc_idx].pending_map_cnt > 0 && 											/* Terminated task freed desired processor */
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

void tm_migrate(mapper_t *mapper, int task)
{
	Echo("Received migration request to task id "); Echo(itoa(task)); Echo("\n");
}
