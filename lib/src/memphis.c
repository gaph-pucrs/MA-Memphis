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

int memphis_get_id()
{
	return system_call(GETID, 0, 0, 0);
}

int memphis_get_addr()
{
	return system_call(GETLOCATION, 0, 0, 0);
}

int memphis_send(message_t *msg, int target_id)
{
	while(!system_call(WRITEPIPE, msg, target_id, 0));
	return 0;
}

int memphis_receive(message_t *msg, int source_id)
{
	while(!system_call(READPIPE, msg, source_id, 0));
	return 0;
}

int memphis_get_tick()
{
	return system_call(GETTICK, 0, 0, 0);
}

int memphis_send_any(message_t *msg, int target_id)
{
	while(!system_call(WRITEPIPE, msg, target_id, 1));
	return 0;
}

int memphis_receive_any(message_t *msg)
{
	while(!system_call(READPIPE, msg, 0, 1));
	return 0;
}

int memphis_real_time(int period, int deadline, int exec_time)
{
	return system_call(REALTIME, period, deadline, exec_time);
}

int memphis_br_send(uint32_t payload, uint16_t target, uint8_t service)
{
	while(!system_call(SCALL_BR_SEND, payload, target, service));
	return 0;
}
