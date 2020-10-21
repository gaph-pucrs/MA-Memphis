/**
 * @file pkg.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @brief Sample kernel PKG file with project time definitions
 * 
 * @details This will be updated by the builder.
 * 
 */

#pragma once

#define PKG_MAX_LOCAL_TASKS		1		//!> Max task allowed to execute into a single processor
#define PKG_PAGE_SIZE			32768	//!> The page size each task will have (inc. kernel)
#define PKG_MAX_TASKS_APP		10		//!> Max number of tasks for the APPs described into testcase file
#define PKG_PENDING_SVC_MAX 	20		//!< Pending service array size
#define PKG_SLACK_TIME_WINDOW	50000	//!< Half millisecond
#define PKG_MAX_KERNEL_MSG_LEN	10		//!< Size of the kernel output pending service

#define PKG_N_PE				9		//!< Number of PEs
#define PKG_N_PE_X				3		//!< Number of PEs in X dimension

#define PKG_PERIPHERALS_NUMBER	1			//!< Number of Peripherals
#define APP_INJECTOR			0xe0000000	//!< Port where the Peripheral App Injector is connected
