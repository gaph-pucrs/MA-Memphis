#include "string.h"

/* https://aticleworld.com/memset-in-c/ */
void *memset(void *ptr, int value, size_t num)
{
	unsigned char *p = ptr;
    
	while(num--)
    {
        *p++ = (unsigned char)value;
    }

    return ptr;
}

/* https://www.techiedelight.com/implement-strcpy-function-c/ */
char *strcpy(char *destination, const char *source)
{
	/* return if no memory is allocated to the destination */
    if(destination == NULL){
        return NULL;
    }
 
    /* take a pointer pointing to the beginning of the destination string */
    char *ptr = destination;
 
    /* copy the C-string pointed by source into the array */
    /* pointed by destination */
    while(*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    /* include the terminating null character */
    *destination = '\0';
 
    /* the destination is returned by standard `strcpy()` */
    return ptr;
}

size_t strlen(const char *str)
{
	const char *s = str;
	
	while(*s++);

	return s - str;
}
