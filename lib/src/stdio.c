/**
 * MA-Memphis
 * @file stdio.c
 *
 * @author Angelo Elias Dalzotto (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 *
 * @brief stdio library for MA-Memphis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memphis.h"

#include "mmr.h"
#include "printf.h"

int puts(const char *str)
{
	int ret = 0;
	if(_has_priv){
		MMR_UART_DATA = (unsigned int)str;
	} else {
		ret |= system_call(SCALL_PUTS, str, 0, 0);
	}

	return ret;
}

int printf(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	const int ret = vprintf(format, va);
	va_end(va);

	return ret;
}

int vprintf(const char *format, va_list arg)
{
	char buffer[256] = {};

	int ret = vsnprintf_(buffer, 256, format, arg);

	ret |= puts(buffer);

	return ret;
}

void _putchar(char character)
{
	if(_has_priv){
		MMR_UART_CHAR = character;
	} else {
		system_call(SCALL_PUTC, (int)character, 0, 0);
	}
}
