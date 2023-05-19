/**
 * MA-Memphis
 * @file mmr.h
 * 
 * @author Unknown
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date Unknown
 * 
 * @brief Define memory mapped registers adressing.
 */

#pragma once

#define MMR_UART_DATA 				(*(volatile unsigned int*)0x20000000U)	//!< Read/Write data from/to UART
#define MMR_UART_CHAR 				(*(volatile unsigned int*)0x20000004U)	//!< Read/Write data from/to UART
#define MMR_UART_LEN 				(*(volatile unsigned int*)0x20000008U)	//!< Read/Write data from/to UART
#define MMR_UART_START 				(*(volatile unsigned int*)0x2000000CU)	//!< Read/Write data from/to UART
#define MMR_IRQ_MASK 				(*(volatile unsigned int*)0x20000010U)	//!< NoC interrupt mask
#define MMR_IRQ_STATUS				(*(volatile unsigned int*)0x20000020U)
#define MMR_TIME_SLICE 				(*(volatile unsigned int*)0x20000060U)
#define MMR_TASK_TERMINATED			(*(volatile unsigned int*)0x20000070U)	//!< Signals that a task terminated
#define MMR_END_SIM 				(*(volatile unsigned int*)0x20000080U)
#define MMR_CLOCK_HOLD				(*(volatile unsigned int*)0x20000090U)	//!< Clock hold register
#define MMR_NI_CONFIG				(*(volatile unsigned int*)0x20000140U)	//!< Net address
#define MMR_DMNI_SIZE 				(*(volatile unsigned int*)0x20000200U)	//!< Size of the packet to read
#define MMR_DMNI_SIZE_2 			(*(volatile unsigned int*)0x20000204U)	//!< Size of the packet payload
#define MMR_DMNI_ADDRESS			(*(volatile unsigned int*)0x20000210U)	//!< Address of the variable of the operation
#define MMR_DMNI_ADDRESS_2 			(*(volatile unsigned int*)0x20000214U)	//!< Address of the packet payload
#define MMR_DMNI_OP					(*(volatile unsigned int*)0x20000220U)	//!< Operation request to DMNI
#define MMR_DMNI_START				(*(volatile unsigned int*)0x20000230U)	//!< Signals to start operation
#define MMR_DMNI_SEND_ACTIVE		(*(volatile unsigned int*)0x20000250U)	//!< Signals a transfer is happening
#define MMR_DMNI_RECEIVE_ACTIVE		(*(volatile unsigned int*)0x20000260U)	//!< Signals a transfer is happening
#define MMR_SCHEDULING_REPORT		(*(volatile unsigned int*)0x20000270U)	//!< Report what is happening in scheduler
#define MMR_ADD_PIPE_DEBUG			(*(volatile unsigned int*)0x20000280U)
#define MMR_REM_PIPE_DEBUG 			(*(volatile unsigned int*)0x20000284U)	//!< Produce debug remove pipe
#define MMR_ADD_REQUEST_DEBUG 		(*(volatile unsigned int*)0x20000290U)	//!< Produce debug message request
#define MMR_REM_REQUEST_DEBUG		(*(volatile unsigned int*)0x20000294U)
#define MMR_TICK_COUNTER			(*(volatile unsigned int*)0x20000300U)	//!< CPU Clock ticks counter
#define MMR_SLACK_TIME_MONITOR 		(*(volatile unsigned int*)0x20000370U)
#define MMR_PENDING_SERVICE_INTR	(*(volatile unsigned int*)0x20000400U)	//!< Signals there is a service pending
#define MMR_MEM_REG_PERIPHERALS     (*(volatile unsigned int*)0x20000500U)
#define MMR_BR_LOCAL_BUSY     		(*(volatile unsigned int*)0x20000600U)
#define MMR_BR_PAYLOAD     			(*(volatile unsigned int*)0x20000604U)
#define MMR_BR_TARGET     			(*(volatile unsigned int*)0x20000608U)
#define MMR_BR_SERVICE     			(*(volatile unsigned int*)0x2000060CU)
#define MMR_BR_START     			(*(volatile unsigned int*)0x20000610U)
#define MMR_BR_HAS_MESSAGE			(*(volatile unsigned int*)0x20000614U)
#define MMR_BR_READ_PAYLOAD			(*(volatile unsigned int*)0x20000618U)
#define MMR_MON_PTR_QOS				(*(volatile unsigned int*)0x2000061CU)
#define MMR_MON_PTR_PWR				(*(volatile unsigned int*)0x20000620U)
#define MMR_MON_PTR_2				(*(volatile unsigned int*)0x20000624U)
#define MMR_MON_PTR_3				(*(volatile unsigned int*)0x20000628U)
#define MMR_MON_PTR_4				(*(volatile unsigned int*)0x2000062CU)
#define MMR_BR_PRODUCER				(*(volatile unsigned int*)0x20000630U)
#define MMR_DMNI_CLEAR_MONITOR		(*(volatile unsigned int*)0x20000634U)
#define MMR_BR_READ_PRODUCER		(*(volatile unsigned int*)0x20000638U)
#define MMR_BR_KSVC					(*(volatile unsigned int*)0x2000063CU)
#define MMR_BR_READ_KSVC			(*(volatile unsigned int*)0x20000640U)
#define MMR_BR_POP					(*(volatile unsigned int*)0x20000644U)
#define MMR_PAGE_SIZE				(*(volatile unsigned int*)0x20000648U)
#define MMR_MAX_LOCAL_TASKS			(*(volatile unsigned int*)0x2000064CU)
#define MMR_N_PE_X      			(*(volatile unsigned int*)0x20000650U)
#define MMR_N_PE_Y      			(*(volatile unsigned int*)0x20000654U)
