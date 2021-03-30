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
 * @brief Implements the Low-Level Monitor for Management Application support.
 */

#include "llm.h"

void llm_task(tcb_t *task)
{
	/* What parameters? Deadline? */
	/* Maybe send a full-featured status */
	/* Deadline, execution time, etc. LLM should not process too much info */

	/* How to know where is the configured observer? This info may be contained in task release! */
	/* But how mapper knows where it put the observer? Tag from the Injector? */
}
