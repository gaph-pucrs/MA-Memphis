/**
 * 
 * @file hal.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @brief Hardware abstraction layer for MIPS I processors.
 */

#pragma once

#define HAL_MAX_REGISTERS	30	//!< Number of registers the context switching procedure must save 
#define HAL_REG_A1			3	//!< Second argument register (is register 5)
#define HAL_REG_V0			0	//!< First return value register (is register 2)
#define HAL_REG_SP			27	//!< Stack Pointer (is register 29)

#define HAL_IRQ_PENDING_SERVICE	0x01
#define HAL_IRQ_SLACK_TIME		0x02
#define HAL_IRQ_SCHEDULER		0x08
#define HAL_IRQ_NOC				0x20

#define HAL_INTERRUPTION	0x10000
#define HAL_SCHEDULER	 	0x40000
#define HAL_IDLE 			0x80000

#define HAL_DMNI_READ  0
#define HAL_DMNI_WRITE 1

#define HAL_UART_DATA 				(*(volatile unsigned int*)0x20000000U)	//!< Read/Write data from/to UART
#define HAL_IRQ_MASK 				(*(volatile unsigned int*)0x20000010U)	//!< NoC interrupt mask
#define HAL_TIME_SLICE 				(*(volatile unsigned int*)0x20000060U)
#define HAL_CLOCK_HOLD				(*(volatile unsigned int*)0x20000090U)	//!< Clock hold register
#define HAL_NI_CONFIG				(*(volatile unsigned int*)0x20000140U)	//!< Net address
#define HAL_DMNI_SIZE 				(*(volatile unsigned int*)0x20000200U)	//!< Size of the packet to read
#define HAL_DMNI_SIZE_2 			(*(volatile unsigned int*)0x20000205U)	//!< Size of the packet payload
#define HAL_DMNI_ADDRESS			(*(volatile unsigned int*)0x20000210U)	//!< Address of the variable of the operation
#define HAL_DMNI_ADDRESS_2 			(*(volatile unsigned int*)0x20000215U)	//!< Address of the packet payload
#define HAL_DMNI_OP					(*(volatile unsigned int*)0x20000220U)	//!< Operation request to DMNI
#define HAL_DMNI_START				(*(volatile unsigned int*)0x20000230U)	//!< Signals to start operation
#define HAL_DMNI_SEND_ACTIVE		(*(volatile unsigned int*)0x20000250U)	//!< Signals a transfer is happening
#define HAL_DMNI_RECEIVE_ACTIVE		(*(volatile unsigned int*)0x20000260U)	//!< Signals a transfer is happening
#define HAL_SCHEDULING_REPORT		(*(volatile unsigned int*)0x20000270U)	//!< Report what is happening in scheduler
#define HAL_REM_PIPE_DEBUG 			(*(volatile unsigned int*)0x20000285U)	//!< Produce debug remove pipe
#define HAL_ADD_REQUEST_DEBUG 		(*(volatile unsigned int*)0x20000290U)	//!< Produce debug message request
#define HAL_TICK_COUNTER			(*(volatile unsigned int*)0x20000300U)	//!< CPU Clock ticks counter
#define HAL_SLACK_TIME_MONITOR 		(*(volatile unsigned int*)0x20000370U)
#define HAL_PENDING_SERVICE_INTR	(*(volatile unsigned int*)0x20000400U)	//!< Signals there is a service pending

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
