#include <sys/stat.h>
#include <newlib.h>
#include <errno.h>
#undef errno
extern int errno;


#include <mmr.h>

#define STDOUT_FILENO 1


/* It turns out that older newlib versions use different symbol names which goes
 * against newlib recommendations. Anyway this is fixed in later version.
 */
#if __NEWLIB__ <= 2 && __NEWLIB_MINOR__ <= 5
#	define _sbrk sbrk
#	define _write write
#	define _close close
#	define _lseek lseek
#	define _read read
#	define _fstat fstat
#	define _isatty isatty
#endif

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _write(int file, char *ptr, int len) {
	
	if (file != STDOUT_FILENO) {
		errno = ENOSYS;
		return -1;
    }

	/**
	 * @todo
	 * Use a MMR for LEN and another for PTR
	 * Write whole stream at once, let the simulator handle it.
	 */
	int todo;

	for (todo = 0; todo < len; todo++) {
		MMR_UART_CHAR = *ptr++;
	}

	return len;
}

caddr_t _sbrk(int incr) {
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

int _close(int file) {
	return -1;
}
