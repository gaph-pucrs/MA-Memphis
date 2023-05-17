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

#include <memphis/oda.h>
#include <mutils/lru.h>

/**
 * @brief Checks for a deadline violation that can trigger migration
 * 
 * @param tasks LRU queue of tasks
 * @param actuator Pointer to the ODA actuator
 * @param id ID of the monitored message task
 * @param remaining Slack time remaining for the task
 */
void mt_check_rt(lru_t *tasks, oda_t *actuator, int id, int remaining);
