#include "memphis.h"
#include "services.h"
#include "calls.h"
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
	return system_call(WRITEPIPE, msg, target_id, 0);
}

int memphis_receive(message_t *msg, int source_id)
{
	return system_call(READPIPE, msg, source_id, 0);
}

int memphis_get_tick()
{
	return system_call(GETTICK, 0, 0, 0);
}

int memphis_send_any(message_t *msg, int target_id)
{
	return system_call(WRITEPIPE, msg, target_id, 1);
}

int memphis_receive_any(message_t *msg)
{
	return system_call(READPIPE, msg, 0, 1);
}

int memphis_real_time(int period, int deadline, int exec_time)
{
	return system_call(REALTIME, period, deadline, exec_time);
}
