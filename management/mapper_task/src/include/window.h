#pragma once

#include <stdbool.h>

#include "processor.h"
#include "app.h"

typedef struct _window {
	int x;
	int y;

	int wx;
	int wy;
} window_t;

void window_set_from_center(processor_t *processors, app_t *app, window_t *window, unsigned req_pages, int wx, int wy, bool raise_x);
bool window_has_pages(processor_t *processors, window_t *window, int req_pages);
