#include <stdbool.h>
#include <stddef.h>

#include <api.h>

#define PKG_MAX_TASKS_APP 10
#define PKG_MAX_LOCAL_TASKS 1
#define N_PE 9

#define MAP_SVC_IDX 0
#define MAP_TASK_CNT_IDX 1

#define MAP_NEW_APP_REQ 0
#define MAP_NEW_APP 1
#define MAP_TASK_ALLOCATED 2
#define MAP_TASK_TERMINATED 3

typedef struct _task {
	int id;
	int address;
	unsigned code_sz;
	unsigned data_sz;
	unsigned bss_sz;
	unsigned init_addr;
} task_t;

typedef struct _app {
	int id;
	unsigned task_cnt;
	unsigned allocated_cnt;
	task_t *task[PKG_MAX_TASKS_APP];
} app_t;

typedef struct _processor {
	int addr;
	int free_page_cnt;
	int pending_map_cnt;
} processor_t;

typedef struct _mapper {
	unsigned pending_task_cnt;

	unsigned pending_descr_sz;
	unsigned pending_descr[MSG_SIZE - 3];
	app_t *pending_map_app;

	unsigned fail_map_cnt;

	int available_slots;
	int appid_cnt;
	app_t apps[PKG_MAX_LOCAL_TASKS*N_PE];
	task_t tasks[PKG_MAX_LOCAL_TASKS*N_PE];
	processor_t processors[N_PE];
} mapper_t;

int main()
{
	Echo("Mapper started at "); Echo(itoa(GetTick())); Echo("\n");

	mapper_t mapper;
	map_init(&mapper);

	while(true){
		static Message msg;
		SReceive(&msg);
		/* Check what service has been received */
		switch(msg.msg[MAP_SVC_IDX]){
			case MAP_NEW_APP:
				map_new_app(&mapper, msg.msg[1], &msg.msg[3], msg.msg[2]);
				break;
			case MAP_TASK_ALLOCATED:
				map_task_allocated(&mapper, msg.msg[1]);
				break;
			case MAP_TASK_TERMINATED:
				map_task_terminated(&mapper, msg.msg[1]);
				break;
			default:
				Echo("Invalid service received: "); Echo(itoa(msg.msg[MAP_SVC_IDX])); Echo("\n");
				break;
		}
	}

}

void map_init(mapper_t *mapper)
{
	mapper->available_slots = PKG_MAX_LOCAL_TASKS*N_PE - 1;
	mapper->pending_task_cnt = 0;
	mapper->fail_map_cnt = 0;
	/** @todo Change to 0 when using more management tasks */
	mapper->appid_cnt = 1;
	mapper->pending_map_app = NULL;

	task_init(mapper->tasks);
	app_init(mapper->apps);
	processor_init(mapper->processors);
}

void map_new_app(mapper_t *mapper, unsigned task_cnt, unsigned *descriptor, unsigned desc_sz)
{
	Echo("New app received.\n");
	Echo("Descriptor size: "); Echo(itoa(desc_sz)); Echo("\n");
	Echo("App ID: "); Echo(itoa(mapper->appid_cnt)); Echo("\n");

	if(task_cnt > mapper->available_slots){
		Echo("No available slots.\n");

		/* Save pending app descriptor and try to map on TASK_RELEASE */
		mapper->pending_task_cnt = task_cnt;
		mapper->pending_descr_sz = desc_sz;
		/** @todo memcpy */
		for(int i = 0; i < desc_sz; i++)
			mapper->pending_descr[i] = descriptor[i];
	} else {
		Echo("Mapping application...\n");
		
		app_t *app = map_build_app(mapper, mapper->appid_cnt++, descriptor, task_cnt);

		mapper->fail_map_cnt = map_app_mapping(app, mapper->processors);
		if(!mapper->fail_map_cnt){
			Echo("Mapping success! Requesting task allocation.\n");

			/** @todo Ask for task allocation. One each time or all in one? */
			//Send request
		} else {
			mapper->pending_map_app = app;
		}
	}
}

app_t *map_build_app(mapper_t *mapper, int appid, unsigned *descriptor, unsigned task_cnt)
{
	app_t *app = app_get_free(mapper->apps);

	app->id = appid;
	app->task_cnt = task_cnt;
	app->allocated_cnt = 0;

	for(int i = 0; i < app->task_cnt; i++){
		unsigned task_id = descriptor[i*6] & 0xFF;
		app->task[task_id] = task_get_free(mapper->tasks);

		app->task[task_id]->id = appid << 8 | task_id;	
		app->task[task_id]->address = descriptor[i*6 + 1];
		app->task[task_id]->code_sz = descriptor[i*6 + 2];
		app->task[task_id]->data_sz = descriptor[i*6 + 3];
		app->task[task_id]->bss_sz = descriptor[i*6 + 4];
		app->task[task_id]->init_addr = descriptor[i*6 + 5];
	}

	return app;
}

app_t *app_get_free(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*N_PE; i++)
		if(apps[i].id == -1)
			return &apps[i];

	return NULL;
}

task_t *task_get_free(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*N_PE; i++)
		if(tasks[i].id == -1)
			return &tasks[i];

	return NULL;
}

bool map_app_mapping(app_t *app, processor_t *processors)
{
	bool map_failed = false;

	/* First check for static mapping availability */
	for(int i = 0; i < app->task_cnt; i++){
		int address = app->task[i]->address;
		if(address != -1){
			/* Statically mapped task found */
			/* This is needed because more than 1 task can be statically mapped to the same processor */
			processors[address].pending_map_cnt++;
			if(processors[address].pending_map_cnt > processors[address].free_page_cnt){
				Echo("No available page for statically mapped task "); Echo(itoa(i)); Echo("\n");
				map_failed = true;
			}
		}
	}

	if(map_failed){
		unsigned fail_cnt = 0;
		for(int i = 0; i < N_PE; i++){
			if(processors[i].pending_map_cnt > processors[i].free_page_cnt)
				fail_cnt++;
		}
		/* Return the number of processors that need TASK_TERMINATED to retry map */
		return fail_cnt;
	} else {
		/* First map statically mapped tasks */
		for(int i = 0; i < app->task_cnt; i++){
			int address = app->task[i]->address;
			if(address != -1){
				Echo("Statically mapped task "); Echo(itoa(i)); Echo("at address "); Echo(itoa(address));
				processors[address].pending_map_cnt = 0;
				processors[address].free_page_cnt--;
				app->task[i]->address = address;
			}
		}

		/* Now map dynamic tasks */
		for(int i = 0; i < app->task_cnt; i++){
			if(app->task[i]->address == -1){
				int address = processors_get_first_most_free(processors);
				Echo("Dinamically mapped task "); Echo(itoa(i)); Echo("at address "); Echo(itoa(address));
				processors[address].free_page_cnt--;
				app->task[i]->address = address;
			}
		}

		return 0;
	}
}

void task_init(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*N_PE; i++)
		tasks[i].id = -1;
}

void app_init(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*N_PE; i++)
		apps[i].id = -1;
}

void processor_init(processor_t *processors)
{
	for(int i = 0; i < N_PE; i++){
		processors[i].addr = i;
		processors[i].free_page_cnt = PKG_MAX_TASKS_APP;
		processors[i].pending_map_cnt = 0;
	}

	/* Mapper task temporarily only mapped to 0x0 */
	processors[0].free_page_cnt--;
}

int processors_get_first_most_free(processor_t *processors)
{
	int address = 0;
	for(int i = 1; i < N_PE; i++){
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
		/** @todo Assemble and send task release */
	}
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
	int address = app->task[taskid]->address;

	/* Deallocate */
	app->task[taskid]->id = -1;
	app->task[taskid] = NULL;
	app->allocated_cnt--;
	mapper->available_slots++;
	mapper->processors[address].free_page_cnt++;

	/* All tasks terminated, terminate app */
	if(app->allocated_cnt == 0)
		app->id = -1;

	if(mapper->pending_task_cnt > 0){
		/* Pending NEW_APP */
		if(mapper->available_slots >= mapper->pending_task_cnt){
			/* Resources freed. Map pending task */
			Echo("Mapping application...\n");
		
			app = map_build_app(mapper, mapper->appid_cnt++, mapper->pending_descr, mapper->pending_task_cnt);

			mapper->fail_map_cnt = map_app_mapping(app, mapper->processors);
			if(!mapper->fail_map_cnt){
				Echo("Mapping success! Requesting task allocation.\n");

				/** @todo Ask for task allocation. One each time or all in one? */
				//Send request
			} else {
				mapper->pending_map_app = app;
			}

		}
	} else if(mapper->fail_map_cnt > 0){
		/* Pending mapping */
		if(mapper->processors[address].pending_map_cnt > 0){
			/* Terminated task freed desired processor */
			if(mapper->processors[address].free_page_cnt >= mapper->processors[address].pending_map_cnt){
				/* All slots needed in this processor are freed! */
				mapper->processors[address].pending_map_cnt = 0;
				mapper->fail_map_cnt--;
				if(mapper->fail_map_cnt == 0){
					/* All needed processor slots are freed. Map and allocate now */
					mapper->fail_map_cnt = map_app_mapping(mapper->pending_map_app, mapper->processors);
					if(!mapper->fail_map_cnt){
						Echo("Mapping success! Requesting task allocation.\n");

						/** @todo Ask for task allocation. One each time or all in one? */
						//Send request

						mapper->pending_map_app = NULL;
					}
				}
			}
		}
	}
}
