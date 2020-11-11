#pragma once

#include "mapper.h"

void tm_migrate(mapper_t *mapper, int task_id);
void tm_migration_complete(mapper_t *mapper, int task_id);
