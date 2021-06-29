/**
 * MA-Memphis
 * @file task_migration.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Task migration functions
 */

#pragma once

#include "mapper.h"

/**
 * @brief Handles a TASK_MIGRATION_MAP message
 * 
 * @param mapper Pointer to the mapper task
 * @param task_id ID of the task to migrate
 */
void tm_migrate(mapper_t *mapper, int task_id);

/**
 * @brief Handles a TASK_MIGRATED message
 * 
 * @param mapper Pointer to the map structure
 * @param task_id ID of the migrated task
 */
void tm_migration_complete(mapper_t *mapper, int task_id);
