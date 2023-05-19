/**
 * MA-Memphis
 * @file task_location.h
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief Controls task locations for messaging and mapping
 */

#pragma once

#include <mutils/list.h>

/**
 * @brief Structure to associate task and location 
 */
typedef struct _tl {
    int task;
    int addr;
} tl_t;

/**
 * @brief Finds a task location based on a task ID
 * 
 * @param list Pointer to a list (DATA_AV/MESSAGE_REQUEST)
 * @param task ID of the task
 * @return tl_t* Pointer to the task location, NULL if none
 */
tl_t *tl_find(list_t *list, int task);

/**
 * @brief Removes a task location from a list
 * 
 * @param list Pointer to a list (DATA_AV/MESSAGE_REQUEST)
 * @param tl Pointer to the task location
 */
void tl_remove(list_t *list, tl_t *tl);

/**
 * @brief Constructs and adds a task location to a list
 * 
 * @param list Pointer to a list (DATA_AV/MESSAGE_REQUEST)
 * @param task ID of the task
 * @param addr Address of the task
 * @return tl_t* Pointer to the added task location, NULL if failure
 */
tl_t *tl_emplace_back(list_t *list, int task, int addr);

/**
 * @brief Gets a task from a task location
 * 
 * @param tl Pointer to a task location
 * @return int ID of the task
 */
int tl_get_task(tl_t *tl);

/**
 * @brief Gets an address from a task location
 * 
 * @param tl Pointer to a task location
 * @return int Adress of the task
 */
int tl_get_addr(tl_t *tl);

/**
 * @brief Sends a data available
 * 
 * @param dav Pointer to the data available task location
 * @param cons_task ID of the consumer task
 * @param cons_addr Address of the consumer task
 */
void tl_send_dav(tl_t *dav, int cons_task, int cons_addr);

/**
 * @brief Sends a message request
 * 
 * @param msgreq Pointer to the message request task location
 * @param prod_task ID of the producer task
 * @param prod_addr Address of the producer task
 */
void tl_send_msgreq(tl_t *msgreq, int prod_task, int prod_addr);

/**
 * @brief Sets a task location
 * 
 * @param tl Pointer to the task location
 * @param task ID of the task
 * @param addr Address of the task
 */
void tl_set(tl_t *tl, int task, int addr);
