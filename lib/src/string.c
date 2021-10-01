/**
 * MA-Memphis
 * @file string.c
 *
 * @author Angelo Elias Dalzotto (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 *
 * @brief string library for MA-Memphis
 */

#include <stdint.h>

#include "string.h"

void *memcpy(void *destination, const void *source, size_t num)
{
    /* Copy word-wise */
    uint32_t *dst_word = (uint32_t*)destination;
    const uint32_t *src_word = (const uint32_t*)source;
    size_t len_word = num/4;
    for(size_t i = 0; i < len_word; i++)
        dst_word[i] = src_word[i];
    
    /* Copy halfword-wise */
    num -= len_word*4;
    uint16_t *dst_half = (uint16_t *)&dst_word[len_word];
    const uint16_t *src_half = (const uint16_t *)&src_word[len_word];
    size_t len_half = num/2;
    for(size_t i = 0; i < len_half; i++)
        dst_half[i] = src_half[i];

    /* Copy byte-wise */
    num -= len_half*2;
    uint8_t *dst_byte = (uint8_t *)&dst_half[len_half];
    const uint8_t *src_byte = (const uint8_t *)&src_half[len_half];
    for(size_t i = 0; i < num; i++)
        dst_byte[i] = src_byte[i];

    return destination;
}

/* https://www.techiedelight.com/implement-strcpy-function-c/ */
char *strcpy(char *destination, const char *source)
{
	/* return if no memory is allocated to the destination */
    if(destination == NULL){
        return NULL;
    }
 
    /* take a pointer pointing to the beginning of the destination string */
    char *ptr = destination;
 
    /* copy the C-string pointed by source into the array */
    /* pointed by destination */
    while(*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    /* include the terminating null character */
    *destination = '\0';
 
    /* the destination is returned by standard `strcpy()` */
    return ptr;
}

/* https://aticleworld.com/memset-in-c/ */
void *memset(void *ptr, int value, size_t num)
{
	unsigned char *p = ptr;
    
	while(num--)
    {
        *p++ = (unsigned char)value;
    }

    return ptr;
}

size_t strlen(const char *str)
{
	const char *s = str;
	
	while(*(s++));

	return s - str;
}
