/**
 * MA-Memphis
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

#include <stdbool.h>
#include <stdint.h>

#include "monitor.h"

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
int os_syscall(unsigned int service, unsigned int a1, unsigned int a2, unsigned int a3);

/**
 * @brief Exit the task and deallocate resources
 * 
 * @param status Return status code
 * 
 * @return True if exited. False if it must wait for pipe or dmni.
 */
bool os_exit(int status);

/**
 * @brief Sends a message
 * 
 * @param msg_ptr Pointer to the message structure
 * @param cons_task ID of the consumer task
 * @param sync If it should send a data available message before delivery
 * 
 * @return True if message sent (including stored in pipe). False if must retry
 */
bool os_writepipe(unsigned int msg_ptr, int cons_task, bool sync);

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
bool os_readpipe(unsigned int msg_ptr, int prod_task, bool sync);

/**	
 * @brief Get the tick count	
 * 	
 * @return Value of tick count	
 */	
unsigned int os_get_tick();

/**
 * @brief Configures a task real time
 * 
 * @param period Task period in cycles
 * @param deadline Task deadline in cycles
 * @param exec_time Task execution time in cycles
 * 
 * @return True.
 */
bool os_realtime(unsigned int period, int deadline, unsigned int exec_time);

/**
 * @brief Calls a syscall from a received message (MESSAGE_DELIVERY)
 * 
 * @param message Pointer to the message
 * @param length Length of the message
 * 
 * @return True if should schedule
 */
bool os_kernel_syscall(unsigned int *message, int length);

/**
 * @brief Sends a message delivery from kernel
 * 
 * @param task Consumer task
 * @param addr Consumer address
 * @param size Lenght of the message
 * @param msg Pointer to the message
 * 
 * @return True if should schedule
 */
bool os_kernel_writepipe(int task, int addr, int size, int *msg);

/**
 * @brief Releases peripherals connected to this PE
 * 
 * @return False
 */
bool os_release_peripheral();

/**
 * @brief Gets the net address.
 * 
 * @return Net address value.
 */
int os_get_location();

/**
 * @brief Gets the running complete task ID
 * 
 * @return Task ID (with application ID)
 */
int os_get_id();

/**
 * @brief Put a char to debug 'UART'
 * 
 * @param c Character to put
 * 
 * @return 0
 */
int os_putc(char c);

/**
 * @brief Put a string to debug 'UART'
 * 
 * @param str Pointer to string
 * 
 * @return 0
 */
int os_puts(char *str);

/**
 * @brief Sends a message via broadcast
 * 
 * @param payload Message to send
 * @param target PE address to send the message to
 * @param service Broadcast service (ALL/TARGET)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int os_br_send(uint32_t payload, uint16_t target, uint8_t service);

/**
 * @brief Sets the monitoring table pointer to a observer task
 * 
 * @param table Pointer to table.
 * @param type Monitoring type
 * 
 * @return 0 if success. 1 if unauthorized. 2 if wrong type.
 */
int os_mon_ptr(unsigned* table, enum MONITOR_TYPE type);
