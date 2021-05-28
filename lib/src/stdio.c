#include <stdarg.h>

#include "stdio.h"
#include "stdlib.h"
#include "memphis.h"
#include "calls.h"

#include "mmr.h"
#include "printf.h"

static void print_task_heading()
{
	/* Get values from system before start printing */
	int id = memphis_get_id();
	int addr = memphis_get_addr();

	/* Print task heading */
	printf_("$$$_%dx%d_%d_%d_", addr >> 8, addr & 0xFF, id >> 8, id & 0xFF);
}

static void print_task_trailing()
{
	/* Print trailing \n */
	_putchar('\n');
}

int puts(const char *str)
{
	if(_has_priv){
		MMR_UART_DATA = (unsigned int)str;
	} else {
		print_task_heading();
		system_call(SCALL_PUTS, str, 0, 0);
		print_task_trailing();
	}

	return 0;
}

int printf(const char *format, ...)
{
	if((unsigned int)format >= PKG_PAGE_SIZE*1024){
		print_task_heading();
	}

	va_list va;
	va_start(va, format);
	const int ret = vprintf_(format, va);
	va_end(va);

	if((unsigned int)format >= PKG_PAGE_SIZE*1024){
		print_task_trailing();
	}

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
