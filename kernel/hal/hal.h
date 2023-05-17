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

/* Forward Declaration */
typedef struct _tcb tcb_t;

enum {
	// HAL_REG_ZERO,
	HAL_REG_RA,
	HAL_REG_SP,
	HAL_REG_GP,
	// HAL_REG_TP,
	HAL_REG_T0,
	HAL_REG_T1,
	HAL_REG_T2,
	HAL_REG_S0,
	HAL_REG_S1,
	HAL_REG_A0,
	HAL_REG_A1,
	HAL_REG_A2,
	HAL_REG_A3,
	HAL_REG_A4,
	HAL_REG_A5,
	HAL_REG_A6,
	HAL_REG_A7,
	HAL_REG_S2,
	HAL_REG_S3,
	HAL_REG_S4,
	HAL_REG_S5,
	HAL_REG_S6,
	HAL_REG_S7,
	HAL_REG_S8,
	HAL_REG_S9,
	HAL_REG_S10,
	HAL_REG_S11,
	HAL_REG_T3,
	HAL_REG_T4,
	HAL_REG_T5,
	HAL_REG_T6,

	HAL_MAX_REGISTERS
};

enum {
	HAL_EXC_INST_ADDR_MISALG,
	HAL_EXC_INST_ACCS_FLT,
	HAL_EXC_ILLGL_INST,
	HAL_EXC_BREAKPOINT,
	HAL_EXC_LOAD_ADDR_MISALG,
	HAL_EXC_LOAD_ACCS_FLT,
	HAL_EXC_STORE_ADDR_MISALG,
	HAL_EXC_STORE_ACCS_FLT,
	HAL_EXC_ENV_CALL_FROM_U,
	HAL_EXC_ENV_CALL_FROM_S,
	HAL_EXC_ENV_CALL_FROM_M = 11,
	HAL_EXC_INST_PAGE_FLT,
	HAL_EXC_LOAD_PAGE_FLT,
	HAL_EXC_STORE_PAGE_FLT = 15,
};

/**
 * @brief Handles an exception
 * 
 * @details Abort the running task and schedules a new one
 * 
 * @param cause Interruption cause -- machine dependent
 * @param value Interruption value -- cause dependent
 * @param pc Program counter where the exception occurred
 * 
 * @return Pointer to the scheduled TCB
 */
tcb_t *hal_exception_handler(unsigned cause, unsigned value, unsigned pc);
