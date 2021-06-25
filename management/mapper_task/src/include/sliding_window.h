#pragma once

#include "app.h"
#include "processor.h"
#include "window.h"

void sw_map_app(app_t *app, processor_t *processors);
void sw_map_dynamic(app_t *app, task_t *order[], processor_t *processors, window_t *window);
int sw_map_task(task_t *task, app_t *app, processor_t *processors, window_t *window);
