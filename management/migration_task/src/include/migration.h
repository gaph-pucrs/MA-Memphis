/**
 * MA-Memphis
 * @file migration.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Migration decider functions
 */

#pragma once

#include <oda.h>

typedef struct _migration_task {
	int id;
	int missed_cnt;
} migration_task_t;

typedef struct _migration_ring {
	migration_task_t tasks[PKG_PENDING_SVC_MAX];
	unsigned char head;
	// unsigned char tail;
	// bool empty;
	// bool full;
} migration_ring_t;

/**
 * @brief Initializes the migration structure
 * 
 * @param migration Pointer to the migration ring structure
 */
void migration_init(migration_ring_t *migration);

/**
 * @brief Checks for a deadline violation that can trigger migration
 * 
 * @param migration Pointer to the migration ring structure
 * @param actuator Pointer to the ODA actuator
 * @param id ID of the monitored message task
 * @param remaining Slack time remaining for the task
 */
void migration_check_rt(migration_ring_t *migration, oda_t *actuator, int id, int remaining);

/**
 * @brief Searches for a task in the migration ring
 * 
 * @param migration Pointer to the migration ring structure
 * @param id ID of the task to search
 * 
 * @return Pointer to the task. NULL if not found.
 */
migration_task_t *migration_search_task(migration_ring_t *migration, int id);

/**
 * @brief Inserts a new task into the migration ring
 * 
 * @param migration Pointer to the migration ring structure
 * @param id ID of the task to insert
 * 
 * @return Pointer to the inserted task
 */
migration_task_t *migration_task_insert(migration_ring_t *migration, int id);

/**
 * @brief Increments a miss for a desired task
 * 
 * @param task Pointer to the task structure
 */
void migration_task_inc_miss(migration_task_t *task);

/**
 * @brief Gets the miss count of a task
 * 
 * @param task Pointer to the task structure
 * 
 * @return Number of deadline misses of the task
 */
int migration_task_get_miss(migration_task_t *task);

/**
 * @brief Clears the miss count of a task
 * 
 * @param task Pointer to the task structure
 */
void migration_task_clear(migration_task_t *task);
