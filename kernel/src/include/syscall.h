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

#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/times.h>

#include "monitor.h"

struct __timespec64 {
	int64_t tv_sec;         /* Seconds */
	int32_t tv_nsec;        /* Nanoseconds */
	int32_t __padding;      /* Padding */
};

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
int os_syscall(unsigned service, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5, unsigned arg6, unsigned arg7, unsigned number);

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
 * @return 0 if message sent (including stored in pipe).
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry
 *         -EBADMSG on message protocol errors
 *         -EACCES on unauthorized targets
 */
int os_writepipe(unsigned int msg_ptr, int cons_task, bool sync);

/**
 * @brief Receives a message
 * 
 * @param msg_ptr Pointer to message structure to save to
 * @param prod_task ID of the producer task
 * @param sync If it should wait for a data available message before requesting
 * 
 * @return 0 if read is sucess (including if waiting for delivery). 
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry
 *         -EBADMSG on message protocol errors
 */
int os_readpipe(unsigned int msg_ptr, int prod_task, bool sync);

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
int os_getpid();

/**
 * @brief Sends a message via broadcast
 * 
 * @param payload Message to send
 * @param ksvc Kernel service used in ALL and TARGET (see services.h)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int os_br_send_all(uint32_t payload, uint8_t ksvc);

/**
 * @brief Sends a message via broadcast
 * 
 * @param payload Message to send
 * @param target PE address to send the message to
 * @param ksvc Kernel service used in ALL and TARGET (see services.h)
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int os_br_send_tgt(uint32_t payload, uint16_t target, uint8_t ksvc);

/**
 * @brief Sets the monitoring table pointer to a observer task
 * 
 * @param table Pointer to table.
 * @param type Monitoring type
 * 
 * @return 0 if success. 1 if unauthorized. 2 if wrong type.
 */
int os_mon_ptr(unsigned* table, enum MONITOR_TYPE type);

/**
 * @brief Closes a file
 * 
 * @details There are no valid files for closing.
 * 
 * @param file File number
 * 
 * @return int -1
 */
int os_close(int file);

/**
 * @brief Writes to a file
 * 
 * @param file File number
 * @param buf Pointer of the buffer to write
 * @param nbytes Number of bytes to write
 * 
 * @return int Number of bytes written
 */
int os_write(int file, char *buf, int nbytes);

/**
 * @brief Get status of a file
 * 
 * @param file File number
 * @param st Pointer to stat structure
 * 
 * @return int -1 if invalid file, 0 if valid
 */
int os_fstat(int file, struct stat *st);

/**
 * @brief Change data segment size
 * 
 * @param addr Program break address to set
 * 
 * @return int Program break address on success, -1 on failure
 */
int os_brk(void *addr);

/**
 * @brief Gets time of the day. Currently not implemented.
 * 
 * @param tp Timespec64 structure pointer
 * @param tzp Deprecated timezone. Should be NULL.
 * 
 * @return int -1
 */
int os_clock_gettime64(struct __timespec64 *ts64, void *tzp);
