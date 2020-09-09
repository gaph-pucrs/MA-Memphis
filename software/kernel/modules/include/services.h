/**
 * 
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

const uint32_t MESSAGE_REQUEST = 0x00000010;;
const uint32_t MESSAGE_DELIVERY = 0x00000020;
const uint32_t TASK_ALLOCATION = 0x00000040;
const uint32_t TASK_ALLOCATED = 0x00000050;
const uint32_t TASK_REQUEST = 0x00000060;
const uint32_t TASK_TERMINATED = 0x00000070;
const uint32_t LOAN_PROCESSOR_RELEASE = 0x00000090;
const uint32_t DEBUG_MESSAGE = 0x00000100;
const uint32_t TASK_MAPPING_REPORT = 0x00000130;
const uint32_t APP_TERMINATED = 0x00000140;
const uint32_t NEW_APP = 0x00000150;
const uint32_t INITIALIZE_CLUSTER = 0x00000160;
const uint32_t INITIALIZE_SLAVE = 0x00000170;
const uint32_t TASK_TERMINATED_OTHER_CLUSTER = 0x00000180;
const uint32_t LOAN_PROCESSOR_REQUEST = 0x00000190;
const uint32_t LOAN_PROCESSOR_DELIVERY = 0x00000200;
const uint32_t TASK_MIGRATION = 0x00000210;
const uint32_t MIGRATION_CODE = 0x00000220;
const uint32_t MIGRATION_TCB = 0x00000221;
const uint32_t MIGRATION_TASK_LOCATION = 0x00000222;
const uint32_t MIGRATION_MSG_REQUEST = 0x00000223;
const uint32_t MIGRATION_STACK = 0x00000224;
const uint32_t MIGRATION_DATA_BSS = 0x00000225;
const uint32_t UPDATE_TASK_LOCATION = 0x00000230;
const uint32_t TASK_MIGRATED = 0x00000235;
const uint32_t APP_ALLOCATION_REQUEST = 0x00000240;
const uint32_t TASK_RELEASE = 0x00000250;
const uint32_t SLACK_TIME_REPORT = 0x00000260;
const uint32_t DEADLINE_MISS_REPORT = 0x00000270;
const uint32_t REAL_TIME_CHANGE = 0x00000280;
const uint32_t NEW_APP_REQ = 0x00000290;
const uint32_t APP_REQ_ACK = 0x00000300;
const uint32_t DATA_AV = 0x00000310;

const uint32_t EXIT = 0;
const uint32_t WRITEPIPE = 1;
const uint32_t READPIPE = 2;
const uint32_t GETTICK = 3;
const uint32_t ECHO = 4;
const uint32_t REALTIME = 5;
const uint32_t IOSEND = 6;
const uint32_t IORECEIVE = 7;
const uint32_t APPID = 8;
