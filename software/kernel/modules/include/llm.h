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


// criar aqui
typedef struct powe
{
    int addr=0;
    
    /* data */
};



/**
 * @brief Runs the LLM procedure for a task.
 * 
 * @details
 * Grep the task status and sends to the configured observer task.
 * 
 * @param task Pointer to the desired tcb to be monitored.
 */
void llm_task(tcb_t *task);

/**
 * @brief Run the LLM procedure for RT tasks.
 * 
 * @param tasks TCB array
 */
void llm_rt(tcb_t *tasks);
