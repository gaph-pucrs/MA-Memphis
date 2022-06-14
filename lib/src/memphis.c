/**
 * MA-Memphis
 * @file memphis.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 * 
 * @brief Memphis API
 */

#include "memphis.h"
#include "services.h"
#include "mmr.h"

#include "internal_syscall.h"

int memphis_get_addr()
{
	return __internal_syscall(SYS_getlocation, 0, 0, 0, 0, 0, 0, 0);
}

int memphis_send(message_t *msg, int target_id)
{
	int ret = 0;
	do {
		ret = syscall_errno(SYS_writepipe, 3, (int)msg, target_id, 0, 0, 0, 0);
	} while(ret == -1 && errno == EAGAIN);
	return ret;
}

int memphis_receive(message_t *msg, int source_id)
{
	int ret = 0;
	do {
		ret = syscall_errno(SYS_readpipe, 3, (int)msg, source_id, 0, 0, 0, 0);
	} while(ret == -1 && errno == EAGAIN);
	return ret;
}

unsigned memphis_get_tick()
{
	return __internal_syscall(SYS_gettick, 0, 0, 0, 0, 0, 0, 0);
}

int memphis_send_any(message_t *msg, int target_id)
{
	int ret = 0;
	do {
		ret = syscall_errno(SYS_writepipe, 3, (int)msg, target_id, 1, 0, 0, 0);
	} while(ret == -1 && errno == EAGAIN);
	return ret;
}

int memphis_receive_any(message_t *msg)
{
	int ret = 0;
	do {
		ret = syscall_errno(SYS_readpipe, 3, (int)msg, 0, 1, 0, 0, 0);
	} while(ret == -1 && errno == EAGAIN);
	return ret;
}

int memphis_real_time(int period, int deadline, int exec_time)
{
	return system_call(REALTIME, period, deadline, exec_time);
}

int memphis_br_send_all(uint32_t payload, uint8_t ksvc)
{
	while(!system_call(SCALL_BR_SEND_ALL, payload, ksvc, 0));
	return 0;
}

int memphis_br_send_tgt(uint32_t payload, uint16_t target, uint8_t ksvc)
{
	while(!system_call(SCALL_BR_SEND_TGT, payload, target, ksvc));
	return 0;
}
