/**
 * MA-Memphis
 * @file hal.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date October 2019
 * 
 * @brief Hardware abstraction layer for RISC-V processors.
 */

#pragma once

#define HAL_MAX_REGISTERS	32	//!< Number of registers the context switching procedure must save 
#define HAL_REG_A1			11	//!< Second argument register (is register 11)
#define HAL_REG_SP			2	//!< Stack Pointer (is register 2)

/**
 * @brief Changes context and runs a task
 * 
 * @param task Pointer to the TCB
 */
void hal_run_task(void* task);

/**
 * @brief Globally disables interrupts
 */
void hal_disable_interrupts();
