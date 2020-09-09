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

#include <stdint.h>

#include "hal.h"

const uint8_t PKG_MAX_LOCAL_TASKS = 1;	//!> Max task allowed to execute into a single processor
const hal_word_t PKG_PAGE_SIZE = 32768;	//!> The page size each task will have (inc. kernel)
const uint8_t PKG_MAX_TASKS_APP = 10;	//!> Max number of tasks for the APPs described into testcase file
// const uint16_t PKG_MAX_TASK_LOCATION = PKG_MAX_LOCAL_TASKS * (uint16_t)PKG_MAX_TASKS_APP;
const uint8_t PKG_PENDING_SVC_MAX = 20;	//!< Pending service array size
const uint16_t PKG_SLACK_TIME_WINDOW = 50000; //!< Half millisecond
