#pragma once

#define MEMPHIS_KERNEL_MSG 0x10000000
#define PKG_MAX_MSG_SIZE 128

typedef struct _message {
	unsigned short length;
	unsigned int payload[PKG_MAX_MSG_SIZE];
} message_t;

extern int system_call();
extern const int _has_priv;

int memphis_get_id();
int memphis_get_addr();
int memphis_send(message_t *msg, int target_id);
int memphis_receive(message_t *msg, int source_id);
int memphis_get_tick();
int memphis_send_any(message_t *msg, int target_id);
int memphis_receive_any(message_t *msg);
int memphis_real_time(int period, int deadline, int exec_time);
