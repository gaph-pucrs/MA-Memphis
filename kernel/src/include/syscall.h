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
#include <stddef.h>

#include <sys/stat.h>

#include <monitor.h>

#include "task_control.h"

/**
 * @brief Decodes a syscall
 * 
 * @param arg1 Argument in a0
 * @param arg2 Argument in a1
 * @param arg3 Argument in a2
 * @param arg4 Argument in a3
 * @param arg5 Argument in a4
 * @param arg6 Argument in a5
 * @param arg7 Argument in a6
 * @param number Syscall number
 * @return int 
 */
int os_syscall(
	unsigned arg1, 
	unsigned arg2, 
	unsigned arg3, 
	unsigned arg4, 
	unsigned arg5, 
	unsigned arg6, 
	unsigned arg7, 
	unsigned number
);

/**
 * @brief Exit the task and deallocate resources
 * 
 * @param tcb Pointer to the TCB
 * @param status Return status code
 * 
 * @return 0 if exited, -EAGAIN if not
 */
int os_exit(tcb_t *tcb, int status);

/**
 * @brief Sends a message
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param cons_task ID of the consumer task
 * @param sync If it should send a data available message before delivery
 * 
 * @return Number of bytes sent/stored in pipe/transferred.
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry
 *         -EBADMSG on message protocol errors
 *         -EACCES on unauthorized targets
 */
int os_writepipe(tcb_t *tcb, void *buf, size_t size, int cons_task, bool sync);

/**
 * @brief Receives a message
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to message to save to
 * @param size Size of the allocated buffer
 * @param prod_task ID of the producer task
 * @param sync If it should wait for a data available message before requesting
 * 
 * @return Number of bytes read.
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry (busy/waiting for interruption)
 *         -EBADMSG on message protocol errors
 */
int os_readpipe(tcb_t *tcb, void *buf, size_t size, int prod_task, bool sync);

/**	
 * @brief Get the tick count	
 * 	
 * @return Value of tick count	
 */	
unsigned int os_get_tick();

/**
 * @brief Configures a task real time
 * 
 * @param tcb Pointer to the producer TCB
 * @param period Task period in cycles
 * @param deadline Task deadline in cycles
 * @param exec_time Task execution time in cycles
 * 
 * @return 0.
 */
int os_realtime(tcb_t *tcb, unsigned int period, int deadline, unsigned int exec_time);

/**
 * @brief Calls a syscall from a received message (MESSAGE_DELIVERY)
 * 
 * @param message Pointer to the message
 * @param length Length of the message
 * 
 * @return True if should schedule
 */
bool os_kernel_syscall(unsigned *message, int length);

/**
 * @brief Sends a message delivery from kernel
 * 
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param cons_task Consumer task
 * @param cons_addr Consumer address
 * 
 * @return True if should schedule
 */
bool os_kernel_writepipe(void *buf, size_t size, int cons_task, int cons_addr);

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
int os_getpid();

/**
 * @brief Sends a message via broadcast
 * 
 * @param tcb Pointer to the producer TCB
 * @param payload Message to send
 * @param ksvc Kernel service used in ALL and TARGET (see services.h)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int os_br_send_all(tcb_t *tcb, uint32_t payload, uint8_t ksvc);

/**
 * @brief Sends a message via broadcast
 * 
 * @param tcb Pointer to the producer TCB
 * @param payload Message to send
 * @param target PE address to send the message to
 * @param ksvc Kernel service used in ALL and TARGET (see services.h)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int os_br_send_tgt(tcb_t *tcb, uint32_t payload, uint16_t target, uint8_t ksvc);

/**
 * @brief Sets the monitoring table pointer to a observer task
 * 
 * @param tcb Pointer to the TCB
 * @param table Pointer to table.
 * @param type Monitoring type
 * 
 * @return 0 if success. 1 if unauthorized. 2 if wrong type.
 */
int os_mon_ptr(tcb_t *tcb, unsigned* table, enum MONITOR_TYPE type);

/**
 * @brief Sets the brk (heap end) of a task
 * 
 * @param tcb Pointer to the TCB
 * @param addr Address to set
 * @return int Address of the new heap end if modified, previous heap end if
 * unmodified, and -1 if error
 */
int os_brk(tcb_t *tcb, void *addr);

/**
 * @brief Writes to a file
 * 
 * @param tcb Pointer to the TCB
 * @param file File number
 * @param buf Pointer of the buffer to write
 * @param nbytes Number of bytes to write
 * 
 * @return int Number of bytes written
 */
int os_write(tcb_t *tcb, int file, char *buf, int nbytes);

/**
 * @brief Get status of a file
 * 
 * @param tcb Pointer to the TCB
 * @param file File number
 * @param st Pointer to stat structure
 * 
 * @return int -1 if invalid file, 0 if valid
 */
int os_fstat(tcb_t *tcb, int file, struct stat *st);

/**
 * @brief Closes a file
 * 
 * @details There are no valid files for closing.
 * 
 * @param file File number
 * 
 * @return int -EBADF
 */
int os_close(int file);
