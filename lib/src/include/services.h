/**
 * MA-Memphis
 * @file services.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 *
 * @brief Kernel services definitions used to identify a packet
 */

#pragma once

/* Messages */
static const unsigned MESSAGE_REQUEST 				= 0x00000000;
static const unsigned MESSAGE_DELIVERY				= 0x00000001;
static const unsigned TASK_ALLOCATED				= 0x00000003;
static const unsigned TASK_REQUEST					= 0x00000004;
static const unsigned LOAN_PROCESSOR_RELEASE		= 0x00000006;
static const unsigned DEBUG_MESSAGE					= 0x00000007;
static const unsigned TASK_MAPPING_REPORT			= 0x00000008;
static const unsigned APP_TERMINATED				= 0x00000009;
static const unsigned NEW_APP						= 0x00000010;
static const unsigned MA_ALLOCATION					= 0x00000011;
static const unsigned INITIALIZE_CLUSTER			= 0x00000012;
static const unsigned INITIALIZE_SLAVE				= 0x00000013;
static const unsigned TASK_TERMINATED_OTHER_CLUSTER	= 0x00000014;
static const unsigned LOAN_PROCESSOR_REQUEST		= 0x00000015;
static const unsigned LOAN_PROCESSOR_DELIVERY		= 0x00000016;
static const unsigned TASK_MIGRATION				= 0x00000017;
static const unsigned MIGRATION_CODE				= 0x00000018;
static const unsigned MIGRATION_TCB					= 0x00000019;
static const unsigned MIGRATION_TASK_LOCATION		= 0x00000020;
static const unsigned MIGRATION_MSG_REQUEST			= 0x00000021;
static const unsigned MIGRATION_STACK				= 0x00000022;
static const unsigned MIGRATION_DATA_BSS			= 0x00000023;
static const unsigned UPDATE_TASK_LOCATION			= 0x00000024;
static const unsigned TASK_MIGRATED					= 0x00000025;
static const unsigned APP_ALLOCATION_REQUEST		= 0x00000026;
static const unsigned TASK_RELEASE					= 0x00000027;
static const unsigned SLACK_TIME_REPORT				= 0x00000028;
static const unsigned DEADLINE_MISS_REPORT			= 0x00000029;
static const unsigned REAL_TIME_CHANGE				= 0x00000030;
static const unsigned DATA_AV						= 0x00000031;
static const unsigned MIGRATION_DATA_AV				= 0x00000032;
static const unsigned MIGRATION_PIPE				= 0x00000033;
static const unsigned APP_MAPPING_COMPLETE			= 0x00000034;
static const unsigned TASK_MIGRATION_MAP			= 0x00000035;
static const unsigned RELEASE_PERIPHERAL			= 0x00000036;
static const unsigned MONITOR						= 0x00000037;
static const unsigned REQUEST_SERVICE				= 0x00000038;
static const unsigned SERVICE_PROVIDER				= 0x00000039;
static const unsigned TASK_ALLOCATION				= 0x00000040;
static const unsigned OBSERVE_PACKET				= 0x00000041;
static const unsigned CLEAR_MON_TABLE				= 0x00000042;
static const unsigned ANNOUNCE_MONITOR				= 0x00000043;
static const unsigned MIGRATION_HEAP				= 0x00000044;
static const unsigned MIGRATION_SCHED				= 0x00000045;
static const unsigned TASK_ABORTED					= 0x00000066;
static const unsigned ABORT_TASK					= 0x00000069;
static const unsigned TASK_TERMINATED				= 0x00000070;
