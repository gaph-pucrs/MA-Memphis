#pragma once

#define ODA_USER		0x01
#define ODA_ACT			0x02
#define ODA_DECIDE		0x04
#define ODA_OBSERVE 	0x08

#define O_QOS		0x0100

#define D_QOS		0x010000

#define A_MIGRATION	0x01000000

enum MON_TYPE {
	MON_QOS,
	MON_PWR,
	MON_2,
	MON_3,
	MON_4
};

typedef struct _monitor {
	int32_t task;	/* In reality 2 bytes, but lets avoid padding problems */
	uint32_t value;
} monitor_t;
