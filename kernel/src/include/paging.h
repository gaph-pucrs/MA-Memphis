/**
 * MA-Memphis
 * @file paging.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (https://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Memphis paging control
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Stores the state of a page 
 */
typedef struct _page {
    void *offset;
    bool free;
} page_t;

/**
 * @brief Initializes pages 
 */
void page_init();

/**
 * @brief Acquire a page
 * 
 * @return page_t* Pointer to a page
 */
page_t *page_acquire();

/**
 * @brief Releases a page
 * 
 * @param page Pointer to page
 */
void page_release(page_t *page);

/**
 * @brief Gets the offset of a page
 * 
 * @param page Pointer to page
 * @return void* Offset of the page
 */
void *page_get_offset(page_t *page);
