#pragma once

#define MMR_IRQ_PENDING_SERVICE	0x01
#define MMR_IRQ_SLACK_TIME		0x02
#define MMR_IRQ_SCHEDULER		0x08
#define MMR_IRQ_NOC				0x20

#define MMR_INTERRUPTION	0x10000
#define MMR_SCHEDULER	 	0x40000
#define MMR_IDLE 			0x80000

#define MMR_DMNI_READ  0
#define MMR_DMNI_WRITE 1

#define MMR_BR_SVC_TGT 1
#define MMR_BR_SVC_ALL 2

#define MMR_UART_DATA 				(*(volatile unsigned int*)0x20000000U)	//!< Read/Write data from/to UART
#define MMR_UART_CHAR 				(*(volatile unsigned int*)0x20000001U)	//!< Read/Write data from/to UART
#define MMR_IRQ_MASK 				(*(volatile unsigned int*)0x20000010U)	//!< NoC interrupt mask
#define MMR_TIME_SLICE 				(*(volatile unsigned int*)0x20000060U)
#define MMR_TASK_TERMINATED			(*(volatile unsigned int*)0x20000070U)	//!< Signals that a task terminated
#define MMR_CLOCK_HOLD				(*(volatile unsigned int*)0x20000090U)	//!< Clock hold register
#define MMR_NI_CONFIG				(*(volatile unsigned int*)0x20000140U)	//!< Net address
#define MMR_DMNI_SIZE 				(*(volatile unsigned int*)0x20000200U)	//!< Size of the packet to read
#define MMR_DMNI_SIZE_2 			(*(volatile unsigned int*)0x20000205U)	//!< Size of the packet payload
#define MMR_DMNI_ADDRESS			(*(volatile unsigned int*)0x20000210U)	//!< Address of the variable of the operation
#define MMR_DMNI_ADDRESS_2 			(*(volatile unsigned int*)0x20000215U)	//!< Address of the packet payload
#define MMR_DMNI_OP					(*(volatile unsigned int*)0x20000220U)	//!< Operation request to DMNI
#define MMR_DMNI_START				(*(volatile unsigned int*)0x20000230U)	//!< Signals to start operation
#define MMR_DMNI_SEND_ACTIVE		(*(volatile unsigned int*)0x20000250U)	//!< Signals a transfer is happening
#define MMR_DMNI_RECEIVE_ACTIVE		(*(volatile unsigned int*)0x20000260U)	//!< Signals a transfer is happening
#define MMR_SCHEDULING_REPORT		(*(volatile unsigned int*)0x20000270U)	//!< Report what is happening in scheduler
#define MMR_REM_PIPE_DEBUG 			(*(volatile unsigned int*)0x20000285U)	//!< Produce debug remove pipe
#define MMR_ADD_REQUEST_DEBUG 		(*(volatile unsigned int*)0x20000290U)	//!< Produce debug message request
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
