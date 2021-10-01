/**
 * MA-Memphis
 * @file stdio.h
 *
 * @author Angelo Elias Dalzotto (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 *
 * @brief stdio library for MA-Memphis
 */

#pragma once

#include <stdarg.h>

/**
 * @brief Writes the C string pointed by format to the standard output (stdout).
 * 
 * @param format C string that contains the text to be written to stdout.
 * 
 * @return On success, the total number of characters written is returned.
 */
int printf(const char *format, ...);

/**
 * @brief Writes the C string pointed by format to the standard output (stdout),
 * replacing any format specifier in the same way as printf does, but using the
 * elements in the variable argument list identified by arg instead of
 * additional function arguments.
 * 
 * @param format C string that contains a format string that follows the same 
 * specifications as format in printf (see printf for details).
 * @param arg A value identifying a variable arguments list initialized with 
 * va_start.
 * 
 * @return On success, the total number of characters written is returned.
 */
int vprintf(const char *format, va_list arg);

/**
 * @brief Writes the C string pointed by str to the standard output (stdout) and
 * appends a newline character ('\n').
 * 
 * @param str C string to be printed.
 * 
 * @return On success, a non-negative value is returned.
 */
int puts(const char *str);
