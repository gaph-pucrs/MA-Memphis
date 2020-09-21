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

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef uint32_t hal_word_t;	//!< Register size

static volatile hal_word_t *HAL_UART_DATA 				= (volatile hal_word_t*)0x20000000U;	//!< Read/Write data from/to UART
static volatile hal_word_t *HAL_IRQ_MASK	 			= (volatile hal_word_t*)0x20000010U;	//!< NoC interrupt mask
static volatile hal_word_t *HAL_TIME_SLICE 				= (volatile hal_word_t*)0x20000060U;
static volatile hal_word_t *HAL_CLOCK_HOLD				= (volatile hal_word_t*)0x20000090U;	//!< Clock hold register
static volatile hal_word_t *HAL_NI_CONFIG				= (volatile hal_word_t*)0x20000140U;	//!< Net address
static volatile hal_word_t *HAL_DMNI_SIZE 				= (volatile hal_word_t*)0x20000200U;	//!< Size of the packet to read
static volatile hal_word_t *HAL_DMNI_SIZE_2 			= (volatile hal_word_t*)0x20000205U;	//!< Size of the packet payload
static volatile hal_word_t *HAL_DMNI_ADDRESS			= (volatile hal_word_t*)0x20000210U;	//!< Address of the variable of the operation
static volatile hal_word_t *HAL_DMNI_ADDRESS_2 			= (volatile hal_word_t*)0x20000215U;	//!< Address of the packet payload
static volatile hal_word_t *HAL_DMNI_OP					= (volatile hal_word_t*)0x20000220U;	//!< Operation request to DMNI
static volatile hal_word_t *HAL_DMNI_START				= (volatile hal_word_t*)0x20000230U;	//!< Signals to start operation
static volatile hal_word_t *HAL_DMNI_SEND_ACTIVE		= (volatile hal_word_t*)0x20000250U;	//!< Signals a transfer is happening
static volatile hal_word_t *HAL_DMNI_RECEIVE_ACTIVE		= (volatile hal_word_t*)0x20000260U;	//!< Signals a transfer is happening
static volatile hal_word_t *HAL_SCHEDULING_REPORT		= (volatile hal_word_t*)0x20000270U;	//!< Report what is happening in scheduler
static volatile hal_word_t *HAL_REM_PIPE_DEBUG 			= (volatile hal_word_t*)0x20000285U;	//!< Produce debug remove pipe
static volatile hal_word_t *HAL_ADD_REQUEST_DEBUG 		= (volatile hal_word_t*)0x20000290U;	//!< Produce debug message request
static volatile hal_word_t *HAL_TICK_COUNTER			= (volatile hal_word_t*)0x20000300U;	//!< CPU Clock ticks counter
static volatile hal_word_t *HAL_SLACK_TIME_MONITOR 		= (volatile hal_word_t*)0x20000370U;
static volatile hal_word_t *HAL_PENDING_SERVICE_INTR	= (volatile hal_word_t*)0x20000400U;	//!< Signals there is a service pending

void hal_run_task(hal_word_t *task);
void hal_disable_interrupts();
