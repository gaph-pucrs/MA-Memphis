/**
 * MA-Memphis
 * @file stdlib.h
 *
 * @author Angelo Elias Dalzotto (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 *
 * @brief stdlib library for MA-Memphis
 */

#pragma once

/**
 * @brief Convert integer to string (non-standard function)
 * 
 * @param value Value to be converted to a string.
 * @param str Array in memory where to store the resulting null-terminated string.
 * @param base Numerical base used to represent the value as a string, between 2
 * and 36, where 10 means decimal base, 16 hexadecimal, 8 octal, and 2 binary.
 * 
 * @return A pointer to the resulting null-terminated string, same as parameter str.
 */
char *itoa(int value, char *str, int base);

/**
 * @brief Returns the absolute value of parameter n ( /n/ ).
 * 
 * @param n Integral value.
 * 
 * @return The absolute value of n.
 */
int abs(int n);

/**
 * @brief Initialize random number generator
 * 
 * @param seed An integer value to be used as seed by the pseudo-random number 
 * generator algorithm.
 */
void srand(unsigned int seed);

/**
 * @brief Generate random number
 * 
 * @return An integer value between 0 and RAND_MAX.
 */
int rand();

/**
 * @brief Terminate calling process
 * 
 * @param status Status code.
 */
void exit(int status);
