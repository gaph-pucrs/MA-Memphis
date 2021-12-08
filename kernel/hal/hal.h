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
 * @brief Hardware abstraction layer for MIPS I processors.
 */

#pragma once

#define HAL_MAX_REGISTERS	30	//!< Number of registers the context switching procedure must save 
#define HAL_REG_A1			3	//!< Second argument register (is register 5)
#define HAL_REG_V0			0	//!< First return value register (is register 2)
#define HAL_REG_SP			25	//!< Stack Pointer (is register 29)

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
