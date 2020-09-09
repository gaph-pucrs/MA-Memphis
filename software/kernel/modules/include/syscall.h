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
int syscall(hal_word_t service, hal_word_t a1, hal_word_t a2, hal_word_t a3);

/**
 * @brief Exit the task and deallocate resources
 * 
 * @return True if exited. False if it must wait for pipe or dmni.
 */
bool os_exit();
