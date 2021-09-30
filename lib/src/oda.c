/**
 * MA-Memphis
 * @file oda.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Standard function for OD(A) tasks
 */

#include "memphis.h"

#include "oda.h"
#include "services.h"
#include "tag.h"
#include "calls.h"

void oda_init(oda_t *oda)
{
	oda->id = -1;
	oda->tag = -1;
}

void oda_request_service(oda_t *oda, int type_tag)
{
	oda->tag = type_tag;
	message_t msg;

	msg.payload[0] = REQUEST_SERVICE;
	msg.payload[1] = memphis_get_addr();
	msg.payload[2] = oda->tag;
	msg.payload[3] = memphis_get_id();
	msg.length = 4;

	memphis_send_any(&msg, 0);	/* Standard mapper task is ID 0 */
}

bool oda_is_enabled(oda_t *oda)
{
	return oda->id != -1;
}

bool oda_service_provider(oda_t *oda, int type_tag, int id)
{
	if(type_tag != oda->tag)
		return false;
	
	oda->id = id;
	return true;
}

bool oda_set_monitor(monitor_t *monitor, enum MON_TYPE type)
{
	return !system_call(SCALL_MON_PTR, monitor, type, 0);
}
