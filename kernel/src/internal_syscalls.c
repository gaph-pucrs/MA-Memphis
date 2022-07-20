/**
 * MA-Memphis
 * @file internal_syscalls.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2022
 * 
 * @brief Internal syscalls for newlib in Memphis kernel
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <newlib.h>
#include <unistd.h>

#undef errno
extern int errno;

#include <mmr.h>

int _fstat(int file, struct stat *st)
{
	if(file != STDOUT_FILENO && file != STDERR_FILENO){
		errno = EBADF;
		return -1;
    }

	st->st_mode = S_IFCHR;
	return 0;
}

int _write(int file, char *ptr, int len)
{
	if(file != STDOUT_FILENO && file != STDERR_FILENO){
		errno = EBADF;
		return -1;
    }

	MMR_UART_LEN = len;
	MMR_UART_DATA = (unsigned)ptr;
	MMR_UART_START = 1;

	return len;
}

caddr_t _sbrk(int incr)
{
	extern char _end;		/* Defined by the linker */
	static char *heap_end;
	char *prev_heap_end;

	/* Initialize heap_end on first call */
	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;

	/* Little trick to get the stack pointer */
	void* stack_ptr = NULL;

	if ((heap_end + incr) > (char*)&stack_ptr) {
		_write(1, "Heap and stack collision\n", 25);
		return NULL;
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

int _close(int file)
{
	return -1;
}

int _gettimeofday(struct timeval *tp, void *tzp)
{
	return -1;
}
