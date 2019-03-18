/*!\file communication.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief Defines the PipeSlot and MessageRequest structures.
 *
 * \detailed
 * PipeSlot stores the user's messages produced by not consumed yet.
 * MessageRequest stores the requested messages send to the consumer task by not produced yet
 */


#ifndef SOFTWARE_INCLUDE_COMMUNICATION_COMMUNICATION_H_
#define SOFTWARE_INCLUDE_COMMUNICATION_COMMUNICATION_H_

#include "../../include/kernel_pkg.h"
#include "../include/api.h"


#define PIPE_SIZE       MAX_LOCAL_TASKS * 3 //24				//!< Size of the pipe array in fucntion of the maximum number of local task
#define REQUEST_SIZE	 MAX_LOCAL_TASKS*(MAX_TASKS_APP-1) //50	//!< Size of the message request array in fucntion of the maximum number of local task and max task per app
#define MAX_TASK_SLOTS	 PIPE_SIZE/MAX_LOCAL_TASKS				//!< Maximum number of pipe slots that a task have

/**
 * \brief This enum stores the pipe status
 */
enum PipeSlotStatus {EMPTY, LOCKED, USED};

/**
 * \brief This structure store a task message (Message) in a kernel memory area called PIPE
 */
typedef struct {
	int producer_task;			//!< Stores producer task id (task that performs the Send() API )
	int consumer_task;			//!< Stores consumer task id (task that performs the Receive() API )
	Message message;			//!< Stores the message itself - Message is a structure defined into api.h
	char status;				//!< Stores pipe status
	unsigned int order;		//!< Stores pipe message order, useful to sort the messages stored in the pipe
} PipeSlot;


/**
 * \brief This structure stores the message requests used to implement the blocking Receive MPI
 */
typedef struct {
    int requester;             	//!< Store the requested task id ( task that performs the Receive() API )
    int requested;             	//!< Stores the requested task id ( task that performs the Send() API )
    int requester_proc;			//!< Stores the requester processor address
} MessageRequest;



void init_communication();

int add_PIPE(int, int, Message *);

unsigned int search_PIPE_producer(int);

unsigned int PIPE_msg_number();

PipeSlot * remove_PIPE(int,  int);

PipeSlot * get_PIPE_free_position();

int insert_message_request(int, int, int);

int search_message_request(int, int);

MessageRequest * remove_message_request(int, int);

int remove_all_requested_msgs(int, unsigned int *);




#endif /* SOFTWARE_INCLUDE_COMMUNICATION_COMMUNICATION_H_ */
