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

#include <stdint.h>

typedef uint32_t hal_word_t;	//!< Register size

const hal_word_t HAL_MAX_REGISTERS = 30;	//!< Number of registers the context switching procedure must save 
const hal_word_t HAL_REG_A1 = 3;			//!< Second argument register
const hal_word_t HAL_REG_V0 = 0;			//!< First return value register

const hal_word_t HAL_IRQ_PENDING_SERVICE = 0x01;
const hal_word_t HAL_IRQ_SLACK_TIME = 0x02;
const hal_word_t HAL_IRQ_SCHEDULER = 0x08;
const hal_word_t HAL_IRQ_NOC = 0x20;

const hal_word_t HAL_INTERRUPTION = 0x10000;
const hal_word_t HAL_IDLE = 0x80000;

const hal_word_t HAL_DMNI_READ = 0;
const hal_word_t HAL_DMNI_WRITE = 1;

volatile hal_word_t *HAL_CLOCK_HOLD		= 0x20000090;	//!< Clock hold register
volatile hal_word_t *HAL_NI_CONFIG		= 0x20000140;	//!< Net address
volatile hal_word_t *HAL_TICK_COUNTER	= 0x20000300;	//!< CPU Clock ticks counter
volatile hal_word_t *HAL_IRQ_MASK 		= 0x20000010;	//!< NoC interrupt mask
volatile hal_word_t *HAL_SCHEDULING_REPORT	= 0x20000270;	//!< Report what is happening in scheduler
volatile hal_word_t *HAL_DMNI_SIZE 		= 0x20000200;	//!< Size of the packet to read
volatile hal_word_t *HAL_DMNI_OP		= 0x20000220;	//!< Operation request to DMNI
volatile hal_word_t *HAL_DMNI_ADDRESS	= 0x20000210;	//!< Address of the variable of the operation
volatile hal_word_t *HAL_DMNI_START		= 0x20000230;	//!< Signals to start operation
volatile hal_word_t *HAL_DMNI_RECEIVE_ACTIVE	= 0x20000260;	//!< Signals a transfer is happening
volatile hal_word_t *HAL_DMNI_SEND_ACTIVE	= 0x20000250;	//!< Signals a transfer is happening
volatile hal_word_t *HAL_PENDING_SERVICE_INTR	= 0x20000400;	//!< Signals there is a service pending
volatile hal_word_t *HAL_DMNI_SIZE_2 	= 0x20000205;	//!< Size of the packet payload
volatile hal_word_t *HAL_DMNI_ADDRESS_2 = 0x20000215;	//!< Address of the packet payload
volatile hal_word_t *HAL_ADD_REQUEST_DEBUG 	= 0x20000290;	//!< Produce debug message request
volatile hal_word_t *HAL_REM_PIPE_DEBUG = 0x20000285;	//!< Produce debug remove pipe
volatile hal_word_t *HAL_SLACK_TIME_MONITOR = 0x20000370;