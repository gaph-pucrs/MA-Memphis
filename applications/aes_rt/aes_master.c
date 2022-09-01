/*********************************************************************
* Filename:   aes_main.c
* Author:     Leonardo Rezende Juracy and Luciano L. Caimi
* Copyleft:   Original AES implementation: Brad Conte 
* 				(brad AT bradconte.com) 
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:   
*********************************************************************/

/*************************** HEADER FILES ***************************/
#include <stdlib.h>
#include <memphis.h>
#include <stdio.h>
#include <unistd.h>
#include "aes_master.h"
/***************************** DEFINES ******************************/
// total message length
#define MSG_LENGHT 256			
// number of efectived used slaves
#define NUMBER_OF_SLAVES 8	
// number of total slaves allocated
#define MAX_SLAVES 8		 	

/**************************** VARIABLES *****************************/

//index of slaves (slave names)
int Slave[MAX_SLAVES] = {aes_slave1,aes_slave2,aes_slave3,aes_slave4,aes_slave5,aes_slave6,aes_slave7,aes_slave8};
unsigned char msg[AES_BLOCK_SIZE];

/*************************** MAIN PROGRAM ***************************/

int main()
{
	volatile int x, y, i,j;
	unsigned char plain_msg[MSG_LENGHT];
	unsigned char cipher_msg[MSG_LENGHT];
#ifdef debug_comunication_on
	int decipher_msg[MSG_LENGHT];
#endif
	int blocks, qtd_messages[MAX_SLAVES];
	int aux_msg[3];
	int aux1_blocks_PE;
	int aux2_blocks_PE;	

	// fill each block with values 'A', 'B', ...
	for(x = 0; x < MSG_LENGHT; x++){
		plain_msg[x] = ((x/16)%26)+0x41;
	}
	
    printf("task AES started %d\n", memphis_get_tick());

	// calculate number of block and pad value (PCKS5) of last block
	// msg_length = MSG_LENGHT;
	blocks = (MSG_LENGHT%AES_BLOCK_SIZE)==0 ? (MSG_LENGHT/AES_BLOCK_SIZE) : (MSG_LENGHT/AES_BLOCK_SIZE)+1;
	// pad_value = (AES_BLOCK_SIZE - (msg_length%AES_BLOCK_SIZE))%AES_BLOCK_SIZE;	
	
	printf("Blocks: %d\n", blocks);

#ifdef debug_comunication_on
    puts("plain msg: ");
    for(x=0; x<MSG_LENGHT-1;x++){
		printf("%x ", plain_msg[x]);
	}
#endif

	//	Calculate number of blocks/messages to sent
	//   to each Slave_PE
	aux1_blocks_PE = blocks / NUMBER_OF_SLAVES;
	aux2_blocks_PE = blocks % NUMBER_OF_SLAVES;
	
	////////////////////////////////////////////////
	//				Start Encrypt				  //
	////////////////////////////////////////////////	
	for(x = 0; x < MAX_SLAVES; x++){
		qtd_messages[x] = aux1_blocks_PE;
		if(x < aux2_blocks_PE)
			qtd_messages[x] += 1;
	}
	
	// Send number of block and operation mode and ID
	// to each Slave_PE
	for(x=0; x < MAX_SLAVES; x++){
		aux_msg[0] = CIPHER_MODE;
		aux_msg[1] = qtd_messages[x];
		aux_msg[2] = x+1;
		if(x >= NUMBER_OF_SLAVES) // zero messages to Slave not used
			aux_msg[0] = END_TASK;
		memphis_send(aux_msg, sizeof(aux_msg), Slave[x]);  
	}

	// Send blocks to Cipher and 
	// Receive the correspondent block Encrypted
	for(x = 0; x < blocks+1; x += NUMBER_OF_SLAVES){
		// send a block to Slave_PE encrypt
		for(y = 0; y < NUMBER_OF_SLAVES; y++){
			if(qtd_messages[(x+y) % NUMBER_OF_SLAVES] != 0){
				memphis_send(&plain_msg[(x+y)*AES_BLOCK_SIZE], AES_BLOCK_SIZE, Slave[(x+y) % NUMBER_OF_SLAVES]);
			}
		}
	
		// Receive Encrypted block from Slave_PE
		for(y = 0; y < NUMBER_OF_SLAVES; y++){
			if(qtd_messages[(x+y) % NUMBER_OF_SLAVES] != 0){
				memphis_receive(msg, AES_BLOCK_SIZE, Slave[(x+y) % NUMBER_OF_SLAVES]);
				j = 0;
				for (i=(x+y)*AES_BLOCK_SIZE;i < ((x+y)*AES_BLOCK_SIZE) + AES_BLOCK_SIZE; i++)
				{
					cipher_msg[i] = msg[j];
					j++;
				}
				j = 0;
				qtd_messages[(x+y) % NUMBER_OF_SLAVES]--;
			}
		}
	}
#ifdef debug_comunication_on
	puts("cipher msg: \n");
	for(i=0; i<MSG_LENGHT;i++){
		printf("%x ", cipher_msg[i]);
	}
#endif 
	
	////////////////////////////////////////////////
	//				Start Decrypt				  //
	////////////////////////////////////////////////
	for(x = 0; x < NUMBER_OF_SLAVES; x++){
		qtd_messages[x] = aux1_blocks_PE;
		if(x <= aux2_blocks_PE)
			qtd_messages[x] += 1;
	}
	
	// Send number of block and operation mode
	// to each Slave_PE
	for(x=0; x < NUMBER_OF_SLAVES; x++){
		aux_msg[0] = DECIPHER_MODE;
		aux_msg[1] = qtd_messages[x];
		memphis_send(aux_msg, sizeof(aux_msg), Slave[x]);  
	}

	// Send blocks to Cipher and 
	// Receive the correspondent block Encrypted
	for(x = 0; x < blocks+1; x += NUMBER_OF_SLAVES){
		// send each block to a Slave_PE
		for(y = 0; y < NUMBER_OF_SLAVES; y++){
			if(qtd_messages[(x+y) % NUMBER_OF_SLAVES] != 0){
				memphis_send(&cipher_msg[(x+y)*AES_BLOCK_SIZE], AES_BLOCK_SIZE, Slave[(x+y) % NUMBER_OF_SLAVES]);   
			} 
		}
		// Receive Encrypted block from Slave_PE
		for(y = 0; y < NUMBER_OF_SLAVES; y++){
			if(qtd_messages[(x+y) % NUMBER_OF_SLAVES] != 0){
				memphis_receive(msg, AES_BLOCK_SIZE, Slave[(x+y) % NUMBER_OF_SLAVES]);
				j = 0;
				for (i=(x+y)*AES_BLOCK_SIZE;i < ((x+y)*AES_BLOCK_SIZE) + AES_BLOCK_SIZE; i++)
				{
				#ifdef debug_comunication_on
					decipher_msg[i] = msg.payload[j];
				#endif
					j++;
				}
				j = 0;
				qtd_messages[(x+y) % NUMBER_OF_SLAVES]--;
			}
		}
	}
#ifdef debug_comunication_on	
	puts("decipher msg: \n");
    for(x=0; x<MSG_LENGHT-1;x++){
		printf("%x ", decipher_msg[x]);
	}
#endif
	//  End tasks still running
	//  End Applicattion
	for(x=0; x < NUMBER_OF_SLAVES; x++){
		aux_msg[0] = END_TASK;
		aux_msg[1] = 0;
		memphis_send(aux_msg, sizeof(aux_msg), Slave[x]);  
	}
	printf("task AES finished at %d\n", memphis_get_tick());

#ifdef debug_comunication_on	
	puts("Final Result: \n");
	unsigned int int_aux2 = 0;
    for(x=0; x<MSG_LENGHT;x+=4){
		int_aux2 = decipher_msg[0+x] << 24;
		int_aux2 = int_aux2 | decipher_msg[1+x] << 16;
		int_aux2 = int_aux2 | decipher_msg[2+x] << 8;
		int_aux2 = int_aux2 | decipher_msg[3+x];
		puts((char*)&int_aux2);
		int_aux2 = 0;
	}
#endif 

	return 0;		
}
