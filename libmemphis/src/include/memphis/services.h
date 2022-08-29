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
#define MESSAGE_REQUEST 				 0x00000000
#define MESSAGE_DELIVERY				 0x00000001
#define TASK_ALLOCATED				 0x00000003
#define TASK_REQUEST					 0x00000004
#define LOAN_PROCESSOR_RELEASE		 0x00000006
#define DEBUG_MESSAGE					 0x00000007
#define TASK_MAPPING_REPORT			 0x00000008
#define APP_TERMINATED				 0x00000009
#define NEW_APP						 0x00000010
#define MA_ALLOCATION					 0x00000011
#define INITIALIZE_CLUSTER			 0x00000012
#define INITIALIZE_SLAVE				 0x00000013
#define TASK_TERMINATED_OTHER_CLUSTER	 0x00000014
#define LOAN_PROCESSOR_REQUEST		 0x00000015
#define LOAN_PROCESSOR_DELIVERY		 0x00000016
#define TASK_MIGRATION				 0x00000017
#define MIGRATION_CODE				 0x00000018
#define MIGRATION_TCB					 0x00000019
#define MIGRATION_TASK_LOCATION		 0x00000020
#define MIGRATION_MSG_REQUEST			 0x00000021
#define MIGRATION_STACK				 0x00000022
#define MIGRATION_DATA_BSS			 0x00000023
#define UPDATE_TASK_LOCATION			 0x00000024
#define TASK_MIGRATED					 0x00000025
#define APP_ALLOCATION_REQUEST		 0x00000026
#define TASK_RELEASE					 0x00000027
#define SLACK_TIME_REPORT				 0x00000028
#define DEADLINE_MISS_REPORT			 0x00000029
#define REAL_TIME_CHANGE				 0x00000030
#define DATA_AV						 0x00000031
#define MIGRATION_DATA_AV				 0x00000032
#define MIGRATION_PIPE				 0x00000033
#define APP_MAPPING_COMPLETE			 0x00000034
#define TASK_MIGRATION_MAP			 0x00000035
#define RELEASE_PERIPHERAL			 0x00000036
#define MONITOR						 0x00000037
#define REQUEST_SERVICE				 0x00000038
#define SERVICE_PROVIDER				 0x00000039
#define TASK_ALLOCATION				 0x00000040
#define OBSERVE_PACKET				 0x00000041
#define CLEAR_MON_TABLE				 0x00000042
#define ANNOUNCE_MONITOR				 0x00000043
#define MIGRATION_HEAP				 0x00000044
#define MIGRATION_SCHED				 0x00000045
#define TASK_ABORTED					 0x00000066
#define ABORT_TASK					 0x00000069
#define TASK_TERMINATED				 0x00000070