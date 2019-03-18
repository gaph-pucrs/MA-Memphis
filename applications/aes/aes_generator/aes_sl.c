/*********************************************************************
* Filename:   aes_sl(n).c
* Author:     Leonardo Rezende Juracy and Luciano L. Caimi
* Copyleft:    
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:   
*********************************************************************/

/*************************** HEADER FILES ***************************/
#include <stdlib.h>
#include <api.h>
#include "aes.h"
/**************************** VARIABLES *****************************/

Message msg;

/*************************** MAIN PROGRAM ***************************/

int main()
{
	unsigned int key_schedule[60];
	int qtd_messages, op_mode, x, flag=1, id = -1, i;
	unsigned int enc_buf[128];
	unsigned int input_text[16]; 
	unsigned int key[1][32] = {
		{0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4}
	};

	Echo(itoa(GetTick()));
    Echo("task AES SLAVE started - ID:"); 
	aes_key_setup(&key[0][0], key_schedule, 256);    
    
    while(flag){
		Receive(&msg, aes_master);
		memcpy(input_text, msg.msg, 12);
			
#ifdef debug_comunication_on
	Echo(" ");  
	Echo("Slave configuration");
	for(i=0; i<3;i++){
		Echo(itoh(input_text[i]));		
	}
	Echo(" "); 
#endif 
				
		op_mode = input_text[0];
		qtd_messages = input_text[1];
		x = input_text[2];	
		
		if(id == -1){
				id = x;
				Echo(itoa(id));
		}	
		Echo("Operation:"); 
		Echo(itoa(op_mode));
		Echo("Blocks:"); 		
		Echo(itoa(qtd_messages));

		if (op_mode == END_TASK){
			flag = 0;
			qtd_messages = 0;
		}
		for(x = 0; x < qtd_messages; x++){
			Receive(&msg, aes_master);		
			memcpy(input_text, msg.msg, 4*AES_BLOCK_SIZE);
			
#ifdef debug_comunication_on
	Echo(" ");  
	Echo("received msg");
	for(i=0; i<16;i++){
		Echo(itoh(input_text[i]));		
	}
	Echo(" "); 
#endif 
			
			if(op_mode == CIPHER_MODE){
				Echo("encript");				
				aes_encrypt(input_text, enc_buf, key_schedule, KEY_SIZE);	
			}
			else{
				Echo("decript");					
				aes_decrypt(input_text, enc_buf, key_schedule, KEY_SIZE);
			}			
			msg.length = 4*AES_BLOCK_SIZE;
			memcpy( msg.msg, enc_buf,4*AES_BLOCK_SIZE);
			Send(&msg, aes_master);	
		}
	}
    Echo("task AES SLAVE finished  - ID: ");
    Echo(itoa(id));
    Echo(itoa(GetTick()));
   
	exit();	
}
