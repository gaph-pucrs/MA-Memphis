/*!\file api.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Edited by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * Implements the API for the user's task and defines the structure Message,
 * used by tasks to exchange messages
 */

#ifndef __TASK_H__
#define __TASK_H__

/* Syscalls*/
#define EXIT      			0
#define WRITEPIPE 			1
#define READPIPE  			2
#define GETTICK   			3
#define ECHO      			4
#define	REALTIME			5
#define IOSEND 				6
#define IORECEIVE  			7

#define MemoryWrite(A,V) *(volatile unsigned int*)(A)=(V)
#define TRUE	1
#define FALSE	0

extern int SystemCall();

#define Send(msg, target) while(!SystemCall(WRITEPIPE, (unsigned int*)msg, target,0))
#define Receive(msg, source) while(!SystemCall(READPIPE, (unsigned int*)msg, source,0))
#define SendIO(msg, io_target) while(!SystemCall(IOSEND, (unsigned int*)msg, io_target,0))
#define ReceiveIO(msg, io_source) while(!SystemCall(IOREADPIPE, (unsigned int*)msg, io_source,0))
#define GetTick() SystemCall(GETTICK,0,0,0)
#define Echo(str) SystemCall(ECHO, (char*)str,0,0)
#define exit() while(!SystemCall(EXIT, 0, 0, 0))

//Real-Time API - time represented in microseconds
#define RealTime(period, deadline, execution_time) while(!SystemCall(REALTIME, period, deadline, execution_time))

/*--------------------------------------------------------------------
 * struct Message
 *
 * DESCRIPTION:
 *    Used to handle messages inside the task.
 *    This is not the same structure used in the kernels.
 *
 *--------------------------------------------------------------------*/
//#define MSG_SIZE 128
#define MSG_SIZE 72

typedef struct {
	int length;
	int msg[MSG_SIZE];
} Message;

#endif /*__TASK_H__*/

