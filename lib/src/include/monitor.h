/**
 * MA-Memphis
 * @file monitor.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief Monitoring API using the BrNoC
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

enum MONITOR_TYPE {
	MON_QOS,
	MON_PWR,
	MON_2,
	MON_3,
	MON_4,
	MON_MAX
};

typedef struct _monitor {
	int task;	/* In reality 2 bytes, but lets avoid padding problems */
	int value;
} monitor_t;

#define MONITOR_TABLE(var) volatile static monitor_t var[PKG_N_PE][PKG_MAX_LOCAL_TASKS]

/**
 * @brief Initializes the monitoring table
 * 
 * @param table Table to initialize
 */
void monitor_init(volatile monitor_t table[PKG_N_PE][PKG_MAX_LOCAL_TASKS]);

/**
 * @brief Sets the pointer to the monitor table in the monitoring infrastructure
 * 
 * @param table Table to save the data
 * @param type Monitor class type
 * 
 * @return True if table is set
 */
bool monitor_set_dmni(volatile monitor_t table[PKG_N_PE][PKG_MAX_LOCAL_TASKS], enum MONITOR_TYPE type);

/**
 * @brief Broadcast this task as a monitor
 * 
 * @param type Type of monitoring offered by this task
 */
void monitor_announce(enum MONITOR_TYPE type);
