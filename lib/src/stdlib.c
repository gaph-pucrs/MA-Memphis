#include <stdlib.h>

#include "services.h"
#include "memphis.h"

static unsigned int holdrand;

/* https://www.techiedelight.com/implement-itoa-function-in-c/ */
static void swap(char *x, char *y)
{
	char t = *x;
	*x = *y; 
	*y = t;
}

/* https://www.techiedelight.com/implement-itoa-function-in-c/ */
static char *reverse(char *str, int i, int j)
{
	while(i < j){
		swap(&str[i++], &str[j--]);
	}

	return str;
}

/* https://www.techiedelight.com/implement-itoa-function-in-c/ */
char *itoa(int value, char *str, int base)
{
	/* invalid input */
	if(base < 2 || base > 32){
		return str;
	}
 
	/* consider the absolute value of the number */
	int n = abs(value);
 
	int i = 0;
	while (n){
		int r = n % base;
 
		if(r >= 10){
			str[i++] = 65 + (r - 10);
		} else {
			str[i++] = 48 + r;
		}
 
		n = n / base;
	}
 
	/* if the number is 0 */
	if(i == 0){
		str[i++] = '0';
	}
 
	/**
	 * If the base is 10 and the value is negative, the resulting string
	 * is preceded with a minus sign (-).
	 * With any other base, value is always considered unsigned
	 */
	if(value < 0 && base == 10){
		str[i++] = '-';
	}
 
	str[i] = '\0'; // null terminate string
 
	/* reverse the string and return it */

	return reverse(str, 0, i - 1);
}

int abs(int n)
{
	return n < 0 ? -n : n;
}

/* https://stackoverflow.com/a/4768194 */
void srand(unsigned int seed)
{
	holdrand = seed;
}

/* https://stackoverflow.com/a/4768194 */
int rand()
{
	return(((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

void exit(int status)
{
	system_call(EXIT, status, 0, 0);
}
