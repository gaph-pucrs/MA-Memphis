/****************************** MACROS ******************************/
// The least significant byte of the word is rotated to the end.
#define KE_ROTWORD(x) (((x) << 8) | ((x) >> 24))
#define TRUE  1
#define FALSE 0

//#define debug_comunication_on

#define KEY_SIZE	256
#define AES_BLOCK_SIZE 16        // AES operates on 16 bytes at a time


#define	CIPHER_MODE 	0
#define	DECIPHER_MODE 	1
#define END_TASK		2

#define AES_128_ROUNDS 10
#define AES_192_ROUNDS 12
#define AES_256_ROUNDS 14



