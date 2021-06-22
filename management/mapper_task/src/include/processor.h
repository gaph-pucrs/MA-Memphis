#pragma once

#include <stdbool.h>

typedef struct _processor {
	int addr;
	int free_page_cnt;
	int pending_map_cnt;
	bool failed_map;
} processor_t;

void processor_init(processor_t *processors);
int processors_get_first_most_free(processor_t *processors, int old_proc);
processor_t *processors_get(processor_t *processors, int x, int y);
