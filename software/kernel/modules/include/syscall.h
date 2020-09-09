/**
 * 
 * @file syscall.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Declares the syscall procedures of the kernel.
 */

#pragma once

#include "hal.h"

/**
 * @brief Syscall function call. It choses a service and pass the right arguments
 * 
 * @details This is called by the HAL syscall
 * 
 * @param service	Syscall being called
 * @param a1		Argument in the A1 register
 * @param a2		Argument in the A2 register
 * @param a3		Argument in the A3 register
 */
int os_syscall(hal_word_t service, hal_word_t a1, hal_word_t a2, hal_word_t a3);

/**
 * @brief Exit the task and deallocate resources
 * 
 * @return True if exited. False if it must wait for pipe or dmni.
 */
bool os_exit();

/**
 * @brief Sends a message
 * 
 * @param msg_ptr Pointer to the message structure
 * @param cons_task ID of the consumer task
 * @param sync If it should send a data available message before delivery
 * 
 * @return True if message sent (including stored in pipe). False if must retry
 */
bool os_writepipe(hal_word_t msg_ptr, int cons_task, bool sync);

/**
 * @brief Receives a message
 * 
 * @param msg_ptr Pointer to message structure to save to
 * @param prod_task ID of the producer task
 * @param sync If it should wait for a data available message before requesting
 * 
 * @return True if read is sucess (including if waiting for delivery). False if
 * must retry
 */
bool os_readpipe(hal_word_t msg_ptr, int prod_task, bool sync);

/**
 * @brief Get the tick count
 * 
 * @return Value of tick count
 */
hal_word_t os_get_tick();

/**
 * @brief Gets the application ID of a running task
 * 
 * @return ID of the application (not the ID of the task)
 */
int os_get_appid();

/**
 * @brief Prints a message to the output (log files)
 * 
 * @param msg_ptr Pointer to the message
 * 
 * @return True
 */
bool os_echo(hal_word_t msg_ptr);

/**
 * @brief Configures a task real time
 * 
 * @param period Task period in cycles
 * @param deadline Task deadline in cycles
 * @param exec_time Task execution time in cycles
 * 
 * @return True.
 */
bool os_realtime(hal_word_t period, int deadline, hal_word_t exec_time);
