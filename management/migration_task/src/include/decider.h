#pragma once

#include <stdbool.h>

void decider_request_actor();
bool decider_enabled();
int decider_get_actor();
void decider_service_provider(int tag, int id);