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

/**
 * @brief Runs the LLM procedure.
 * 
 * @details
 * Grep the task status and sends to the configured observer task.
 * 
 * @param task Pointer to the desired tcb to be monitored.
 */
void llm_task(tcb_t *task);
