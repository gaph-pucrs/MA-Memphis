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
