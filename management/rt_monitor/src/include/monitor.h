#pragma once

#include <stdbool.h>

void monitor_request_decider();
bool monitor_enabled();
int monitor_get_decider();
void monitor_service_provider(int tag, int id);
