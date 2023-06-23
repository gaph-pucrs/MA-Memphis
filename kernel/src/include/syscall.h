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

#include <memphis.h>
#include <memphis/monitor.h>

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
 * @return tcb_t* Scheduled tcb 
 */
tcb_t *sys_syscall(
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
int sys_exit(tcb_t *tcb, int status);

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
int sys_writepipe(tcb_t *tcb, void *buf, size_t size, int cons_task, bool sync);

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
int sys_readpipe(tcb_t *tcb, void *buf, size_t size, int prod_task, bool sync);

/**	
 * @brief Get the tick count	
 * 	
 * @return Value of tick count	
 */	
unsigned int sys_get_tick();

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
int sys_realtime(tcb_t *tcb, unsigned int period, int deadline, unsigned int exec_time);

/**
 * @brief Calls a syscall from a received message (MESSAGE_DELIVERY)
 * 
 * @param message Pointer to the message
 * @param length Length of the message
 * 
 * @return True if should schedule
 */
bool sys_kernel_syscall(unsigned *message, int length);

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
bool sys_kernel_writepipe(void *buf, size_t size, int cons_task, int cons_addr);

/**
 * @brief Sends a raw packet
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to the packet
 * @param size Packet size in flits
 * 
 * @return 0 if sent the packet, -EINVAL on invalid argument
 */
int sys_sendraw(tcb_t *tcb, void *buf, size_t size);

/**
 * @brief Releases peripherals connected to this PE
 * 
 * @return False
 */
bool sys_release_peripheral();

/**
 * @brief Gets the net address.
 * 
 * @return Net address value.
 */
int sys_get_location();

/**
 * @brief Gets the running complete task ID
 * 
 * @return Task ID (with application ID)
 */
int sys_getpid();

/**
 * @brief Sends a message via broadcast
 * 
 * @param tcb Pointer to the producer TCB
 * @param payload Message to send
 * @param ksvc Kernel service used in ALL and TARGET (see services.h)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int sys_br_send_all(tcb_t *tcb, uint32_t payload, uint8_t ksvc);

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
int sys_br_send_tgt(tcb_t *tcb, uint32_t payload, uint16_t target, uint8_t ksvc);

/**
 * @brief Sets the monitoring table pointer to a observer task
 * 
 * @param tcb Pointer to the TCB
 * @param table Pointer to table.
 * @param type Monitoring type
 * 
 * @return 0 if success. 1 if unauthorized. 2 if wrong type.
 */
int sys_mon_ptr(tcb_t *tcb, unsigned* table, enum MONITOR_TYPE type);

/**
 * @brief Sets the brk (heap end) of a task
 * 
 * @param tcb Pointer to the TCB
 * @param addr Address to set
 * @return int Address of the new heap end if modified, previous heap end if
 * unmodified, and -1 if error
 */
int sys_brk(tcb_t *tcb, void *addr);

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
int sys_write(tcb_t *tcb, int file, char *buf, int nbytes);

/**
 * @brief Get status of a file
 * 
 * @param tcb Pointer to the TCB
 * @param file File number
 * @param st Pointer to stat structure
 * 
 * @return int -1 if invalid file, 0 if valid
 */
int sys_fstat(tcb_t *tcb, int file, struct stat *st);

/**
 * @brief Closes a file
 * 
 * @details There are no valid files for closing.
 * 
 * @param file File number
 * 
 * @return int -EBADF
 */
int sys_close(int file);

/**
 * @brief Gets the system context
 * 
 * @param tcb Pointer to the TCB
 * @param ctx Pointer to the ctx
 * 
 * @return int 0
 */
int sys_get_ctx(tcb_t *tcb, mctx_t *ctx);

/**
 * @brief Halts the execution
 * 
 * @details In fact, this function will stop the scheduler, free resources, and inform the mapper it is ready to indeed halt
 * 
 * @param tl Task location of the halter (mapper) task
 * 
 * @return int 0 case success, EAGAIN if should retry, EFAULT if impossible to halt.
 */
int sys_halt(tl_t *tl);

/**
 * @brief Ends the simulation
 * 
 * @param tcb TCB of the requester task
 * 
 * @return int 0 if success, EACCES if not permitted
 */
int sys_end_simulation(tcb_t *tcb);
