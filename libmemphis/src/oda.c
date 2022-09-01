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

#include <unistd.h>

#include "memphis.h"

#include "memphis/oda.h"
#include "memphis/services.h"

void oda_init(oda_t *oda)
{
	oda->id = -1;
	oda->tag = -1;
}

void oda_request_service(oda_t *oda, int type_tag)
{
	oda->tag = type_tag;

	int msg[] = {
		REQUEST_SERVICE,
		memphis_get_addr(),
		oda->tag,
		getpid()
	};

	memphis_send_any(&msg, sizeof(msg), 0);	/* Standard mapper task is ID 0 */
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

int oda_get_id(oda_t *oda)
{
	return oda->id;
}
