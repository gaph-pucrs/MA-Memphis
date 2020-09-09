/**
 * 
 * @file interrupts.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Declares the interrupts procedures of the kernel.
 */

#pragma once

#include "hal.h"
#include "packet.h"

/**
 * @brief Function called by the HAL interruption handler.
 * 
 * @details It cannot send a packet when the DMNI is already sending a packet.
 * This function implementation should assure this behavior.
 * 
 * @param status Status of the interruption. Signals the interruption type.
 */
void os_isr(hal_word_t status);

/** 
 * @brief Handles the packet coming from the NoC.
 * 
 * @param packet Pointer to the packet received.
 * 
 * @return True if the scheduler should be called.
 */
bool os_handle_pkt(packet_t *packet);

/**
 * @brief Receive a message request and add to the proper task TCB
 * 
 * @details This is a message synchronization mechanism
 * 
 * @param cons_task Consumer task ID (sender of the packet)
 * @param cons_addr Consumer address (sender of the packet)
 * @param prod_task Producer task ID (receiver of the packet)
 * 
 * @return True if the scheduler should be called.
 */
bool os_message_request(int cons_task, int cons_addr, int prod_task);

/**
 * @brief Receives a message from the NoC
 * 
 * @param cons_task ID of the consumer task
 * @param length Length of the message received
 * 
 * @return True if the scheduler should be called
bool os_message_delivery(int cons_task, hal_word_t length);

/**
 * @brief Handles a data available packet
 * 
 * @param cons_task ID of the consumer task (receiver)
 * @param prod_task ID of the producer task
 * @param prod_addr Address of the producer task
 * 
 * @return True if the scheduler should be called.
 */
bool os_data_available(int cons_task, int prod_task, int prod_addr);

/**
 * @brief Handles a task allocation packet
 * 
 * @param id ID of the new task
 * @param length Code length of the new task
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 * 
 * @return True if the scheduler should be called
 */
bool os_task_allocation(int id, int length, int mapper_task, int mapper_addr);

/**
 * @brief Handles a task release packet
 * 
 * @param id ID of the task to release
 * @param data_sz Length of the data section
 * @param bss_sz Length of the bss section
 * @param task_number Number of the app's tasks
 * 
 * @return True if the scheduler should be called
 */
bool os_task_release(int id, int data_sz, int bss_sz, uint16_t task_number);

/**
 * @brief Handles a update task location packet
 * 
 * @param dest_task ID of the task to be informed of the update
 * @param updt_task	ID of the task that has updated
 * @param updt_addr Address of the task that has updated
 * 
 * @return False
 */
bool os_update_task_location(int dest_task, int updt_task, int updt_addr);
