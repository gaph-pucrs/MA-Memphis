Application: AES
Platform: Memphis 7.3
Authors: Leonardo J. Rezende and Luciano L. Caimi
Data: 2016/06/10
Description: This application works in Master/Slave mode. The application Master
  (aes_master.c) generate a message with size MESSAGE_LENGTH and send each block
  (128 bits) of message to different Slaves (aes_sl1.c, aes_sl2.c, ...) that
  Encrypt each block in parallel. After receive all encrypted blocks the Master
  splits the cipher message and send each cipher block to Slaves Decrypt the
  blocks and recovery the original plaintext message.
  
How to execute the application: to easy simulation was provided the script 
	'generate_aes.sh' that generate the aes_master.c file and all aes_slX.c
	files on directory /application/aes. The script receive 4 arguments:
	$1 - message length in bytes
	$2 - number of total slaves allocated in Memphis MPSoC
	$3 - number of slaves that work in encrypt/decrypt process
	$4 - debug_0/debug_1/debug_2/debug_3 directive
			- debug_0: debug mode off
			- debug_1: debug communication mode on
			- debug_2: debug AES algorithm mode on
			- debug_3: debug communication and AES algorithm mode on
			
	Example:
	./generate_aes.sh 256 8 6 debug_0
	  where: 256 = message length
			 8 = slaves allocated in application
				(generated files: aes_sl1.c, aes_sl2.c, ..., aes_sl8.c)
			 6 = number of slaves that work in encrypt/decrypt process
			 debug_0 = debug mode off. Don't print debug messages
			 
	After execute the script, in the testcase directory the user must execute the
	command:  memphis aes.hmp 20 
	



