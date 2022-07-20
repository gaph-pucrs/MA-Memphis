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
 * @brief Internal syscalls for newlib in Memphis applications
 */

#include <machine/syscall.h>
#include "internal_syscall.h"

int _getpid()
{
	return __internal_syscall(SYS_getpid, 0, 0, 0, 0, 0, 0, 0);
}
