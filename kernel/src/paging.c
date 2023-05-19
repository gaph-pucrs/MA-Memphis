/**
 * MA-Memphis
 * @file paging.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (https://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Memphis paging control
 */

#include "paging.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "mmr.h"

page_t *_pages = NULL;

void page_init()
{
    const unsigned MAX_TASKS = MMR_MAX_LOCAL_TASKS;
    const unsigned PAGE_SIZE = MMR_PAGE_SIZE;

    _pages = malloc(MAX_TASKS*sizeof(page_t));

    if(_pages == NULL){
        puts("FATAL: could not allocate pages structure");
        while(1);
    }

    for(int i = 0; i < MAX_TASKS; i++){
        _pages[i].offset = (void*)(PAGE_SIZE * (i + 1));
        _pages[i].free = true;
    }
}

page_t *page_acquire()
{
    const unsigned MAX_TASKS = MMR_MAX_LOCAL_TASKS;
    for(int i = 0; i < MAX_TASKS; i++){
        if(_pages[i].free){
            _pages[i].free = false;
            return &(_pages[i]);
        }
    }
    return NULL;
}

void page_release(page_t *page)
{
    page->free = true;
}

void *page_get_offset(page_t *page)
{
    return page->offset;
}
