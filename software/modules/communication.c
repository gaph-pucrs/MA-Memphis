/*!\file communication.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief Implements the PIPE and MessageRequest structures management.
 * This module is only used by slave kernel
 */

#include "communication.h"
#include "../include/plasma.h"
#include "utils.h"

PipeSlot pipe[PIPE_SIZE];						//!< pipe array

MessageRequest message_request[REQUEST_SIZE];	//!< message request array

unsigned int pipe_free_positions = PIPE_SIZE;	//!< Stores the number of free position in the pipe


/** Initializes the message request and the pipe array
 */
void init_communication(){
	for(int i=0; i<PIPE_SIZE; i++){
		pipe[i].status = EMPTY;
	}

	for(int i=0; i<REQUEST_SIZE; i++){
		message_request[i].requested = -1;
		message_request[i].requester = -1;
		message_request[i].requester_proc = -1;
	}
}

/** Add a message to the PIPE if it have available space
 *  \param producer_task ID of the producer task
 *  \param consumer_task ID of the consumer task
 *  \param msg Message pointer for the message to be stored
 *  \return 0 if pipe is full, 1 if the message was stored with success
 */
int add_PIPE(int producer_task, int consumer_task, Message * msg){

	PipeSlot * pipe_ptr;
	unsigned int last_order = 0;
	unsigned char task_pipe_slots = 0;

	if (pipe_free_positions == 0){
		return 0;
	}

	for(int i=0; i<PIPE_SIZE; i++){

		pipe_ptr = &pipe[i];

		if (pipe_ptr->status == USED && pipe_ptr->producer_task == producer_task){

			task_pipe_slots++;

			if (pipe_ptr->consumer_task == consumer_task){

				if (last_order < pipe_ptr->order){
					last_order = pipe_ptr->order;
				}
			}
		}
	}

	if (task_pipe_slots == MAX_TASK_SLOTS){
		return 0;
	}

	pipe_ptr = get_PIPE_free_position();

	if (pipe_ptr == 0){
		return 0;
	}

	pipe_ptr->producer_task = producer_task;

	pipe_ptr->consumer_task = consumer_task;

	pipe_ptr->message.length = msg->length;

	pipe_ptr->status = USED;

	pipe_ptr->order = last_order + 1;

	for (int i=0; i<msg->length; i++){
		pipe_ptr->message.msg[i] = msg->msg[i];
	}

	pipe_free_positions--;

	//Only for debug purposes
	MemoryWrite(ADD_PIPE_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

	return 1;

}

/** Tells if the producer task have some message in the pipe
 *  \param producer_task ID of the producer task
 *  \return 0 if it not has, 1 if it has
 */
unsigned int search_PIPE_producer(int producer_task){

	PipeSlot * pipe_ptr;
	unsigned int msg_count=0;

	for(int i=0; i<PIPE_SIZE; i++){

		pipe_ptr = &pipe[i];

		if (pipe_ptr->status == USED && producer_task == pipe_ptr->producer_task){
			msg_count++;
		}
	}

	return msg_count;
}

/** Counts the number of message in the pipe
 *  \return The number of messages in the pipe
 */
unsigned int PIPE_msg_number(){

	PipeSlot * pipe_ptr;
	unsigned int msg_count=0;

	for(int i=0; i<PIPE_SIZE; i++){

		pipe_ptr = &pipe[i];

		if (pipe_ptr->status == USED){
			msg_count++;
		}
	}

	if (msg_count){
		return msg_count;
	}

	return 0;
}

/** Remove the next message from the pipe. The remotion occurs following the order of insertion of the message
 *  \param producer_task ID of the producer task of the message
 *  \param consumer_task ID of the consumer task of the message
 *  \return 0 if it not found any message, or the PipeSlot pointer if the message was successfully removed
 */
PipeSlot * remove_PIPE(int producer_task,  int consumer_task){

	PipeSlot * pipe_ptr, * sel_pipe;
	unsigned int min_order = 0xFFFFFFFF; //Max unsigned integer value

	if (pipe_free_positions == PIPE_SIZE){
		return 0;
	}

	sel_pipe = 0;

	for(int i=0; i<PIPE_SIZE; i++){

		pipe_ptr = &pipe[i];

		if (pipe_ptr->status == USED && producer_task == pipe_ptr->producer_task && consumer_task == pipe_ptr->consumer_task){

			if(min_order > pipe_ptr->order){
				sel_pipe = pipe_ptr;
				min_order = pipe_ptr->order;
			}
		}
	}

	if (sel_pipe == 0){
		return 0;
	}

	sel_pipe->status = EMPTY;

	pipe_free_positions++;

	//Only for debug purposes
	MemoryWrite(REM_PIPE_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

	return sel_pipe;
}


/** Gets a pipe free position pointer
 *  \return PipeSlot free position pointer
 */
PipeSlot * get_PIPE_free_position(){

	for(int i=0; i<PIPE_SIZE; i++){
		if (pipe[i].status == EMPTY){
			return &pipe[i];
		}
	}

	return 0;

}

/** Inserts a message request into the message_request array
 *  \param producer_task ID of the producer task of the message
 *  \param consumer_task ID of the consumer task of the message
 *  \param requester_proc Processor of the consumer task
 *  \return 0 if the message_request array is full, 1 if the message was successfully inserted
 */
int insert_message_request(int producer_task, int consumer_task, int requester_proc) {

    int i;

    for (i=0; i<REQUEST_SIZE; i++)
    	if ( message_request[i].requester == -1 ) {
    		message_request[i].requester  = consumer_task;
    		message_request[i].requested  = producer_task;
    		message_request[i].requester_proc = requester_proc;

    		//Only for debug purposes
    		MemoryWrite(ADD_REQUEST_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

    		return 1;
		}

    puts("ERROR - request table if full\n");
	return 0;	/*no space in table*/
}

/** Searches for a message request
 *  \param producer_task ID of the producer task of the message
 *  \param consumer_task ID of the consumer task of the message
 *  \return 0 if the message was not found, 1 if the message was found
 */
int search_message_request(int producer_task, int consumer_task) {

    for(int i=0; i<REQUEST_SIZE; i++) {
        if( message_request[i].requested == producer_task && message_request[i].requester == consumer_task){
            return 1;
        }
    }

    return 0;
}

/** Remove a message request
 *  \param producer_task ID of the producer task of the message
 *  \param consumer_task ID of the consumer task of the message
 *  \return -1 if the message was not found or the requester processor address (processor of the consumer task)
 */
MessageRequest * remove_message_request(int producer_task, int consumer_task) {

    for(int i=0; i<REQUEST_SIZE; i++) {
        if( message_request[i].requested == producer_task && message_request[i].requester == consumer_task){
        	message_request[i].requester = -1;
        	message_request[i].requested = -1;

        	//Only for debug purposes
        	MemoryWrite(REM_REQUEST_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

            //return message_request[i].requester_proc;
        	return &message_request[i];
        }
    }

    return 0;
}

/**Remove all message request of a requested task ID and copies such messages to the removed_msgs array.
 * This function is used for task migration only, when a task need to be moved to other processor
 *  \param requested_task ID of the requested task
 *  \param removed_msgs array pointer of the removed messages
 *  \return number of removed messages
 */
int remove_all_requested_msgs(int requested_task, unsigned int * removed_msgs){

	int request_index = 0;

	for(int i=0; i<REQUEST_SIZE; i++) {

		if (message_request[i].requested == requested_task){

			//Copies the messages to the array
			removed_msgs[request_index++] = message_request[i].requester;
			removed_msgs[request_index++] = message_request[i].requested;
			removed_msgs[request_index++] = message_request[i].requester_proc;

        	//Only for debug purposes
			MemoryWrite(REM_REQUEST_DEBUG, (message_request[i].requester << 16) | (message_request[i].requested & 0xFFFF));

			//Removes the message request
			message_request[i].requester = -1;
			message_request[i].requested = -1;
		}
	}

	return request_index;
}
