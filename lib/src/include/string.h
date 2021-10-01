/**
 * MA-Memphis
 * @file string.h
 *
 * @author Angelo Elias Dalzotto (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 *
 * @brief string library for MA-Memphis
 */

#pragma once

#include <stddef.h>

/**
 * @brief Copy block of memory
 * 
 * @param destination Pointer to the destination array where the content is to 
 * be copied, type-casted to a pointer of type void*.
 * @param source Pointer to the source of data to be copied, type-casted to a 
 * pointer of type const void*.
 * @param num Number of bytes to copy.
 * 
 * @return destination is returned.
 */
void *memcpy(void *destination, const void *source, size_t num);

/**
 * @brief Copy string
 * 
 * @param destination Pointer to the destination array where the content is to 
 * be copied.
 * @param source C string to be copied.
 * 
 * @return destination is returned.
 */
char *strcpy(char *destination, const char *source);

/**
 * @brief Fill block of memory
 * 
 * @param ptr Pointer to the block of memory to fill.
 * @param value Value to be set. The value is passed as an int, but the function 
 * fills the block of memory using the unsigned char conversion of this value.
 * @param num Number of bytes to be set to the value.
 * 
 * @return ptr is returned.
 */
void *memset(void * ptr, int value, size_t num);

/**
 * @brief Get string length
 * 
 * @param str C string.
 * 
 * @return The length of string.
 */
size_t strlen(const char *str);
