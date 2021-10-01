/**
 * 
 * @file llm.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2020
 * 
 * @brief Declares the Low-Level Monitor for Management Application support.
 */

#include "task_control.h"
#include "monitor.h"

typedef struct _observer {
	int16_t addr;
	uint16_t dist;
} observer_t;

/**
 * @brief Initializes the monitoring structures
 */
void llm_init();

/**
 * @brief Sets an observer if is nearer than the already set
 * 
 * @param type Monitoring type
 * @param addr Address received
 */
void llm_set_observer(enum MONITOR_TYPE type, int addr);

/**
 * @brief Clears the monitoring table of terminated/migrated task
 * 
 * @param task Pointer to the task to clear
 */
void llm_clear_table(tcb_t *task);

/**
 * @brief Run the LLM procedure for RT tasks.
 * 
 * @details
 * Grep the task status and sends to the configured observer task.
 * 
 * @param tasks TCB array
 */
void llm_rt(tcb_t *tasks);
