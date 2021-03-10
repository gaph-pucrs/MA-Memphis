/*!\file stdlib.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Edited by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief  Manual implementation of stdlib  for Memphis
 */

#ifndef __STDLIB_H__
#define __STDLIB_H__

//#include <stdarg.h>
#include<stddef.h>

#define FALSE		0
#define TRUE		1

static
char *itoa(unsigned int num)
{
   static char buf[12];
   static char buf2[12];
   int i,j;

   if (num==0)
   {
      buf[0] = '0';
      buf[1] = '\0';
      return &buf[0];
   }

   for(i=0;i<11 && num!=0;i++)
   {
      buf[i]=(char)((num%10)+'0');
      num/=10;
   }
   buf2[i] = '\0';
   j = 0;
   i--;
   for(;i>=0;i--){
         buf2[i]=buf[j];
         j++;
   }
   return &buf2[0];
}

static
char *itoh(unsigned int num)
{
   static char buf[11];
   int i;
   buf[10]=0;

   buf[0] = '0';
   buf[1] = 'x';

   if (num==0)
   {
      buf[2] = '0';
      buf[3] = '0';
      buf[4] = '0';
      buf[5] = '0';
      buf[6] = '0';
      buf[7] = '0';
      buf[8] = '0';
      buf[9] = '0';
      return buf;
   }

   for(i=9;i>=2;--i)
   {
      if ((num%16) < 10)
         buf[i]=(char)((num%16)+'0');
      else
         buf[i]=(char)((num%16)+'W');

      num/=16;
   }

   return buf;
}

static
int abs(int num)
{
	if(num<0) return -num;
	else return num;
}

static
int rand(int seed, int min, int max)
{ 
	int lfsr = seed;
	
	lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
	
	return (lfsr % max + min);
} 

static
int add(int a, int b)
{
	int res;

	res=a+b;
	return res;
}

static
int sub(int a, int b)
{
	int res;

	res=a-b;
	return res;
}

static
void *memset(void *dst, int c, unsigned long bytes) {

   unsigned char *Dst = (unsigned char*)dst;

   while((int)bytes-- > 0)
      *Dst++ = (unsigned char)c;

   return dst;
}

static
char *fixetoa(int nume)
{
	static char buf[13];
	static char buf2[13];
	int i,j;
	int num=nume;

	if (num<0)
	{
		num=num*(-1);

		for(i=0;i<12 && (num!=0 || i<5) ;i++)
		{
			if (num==0)
				buf[i]='0';
			else
			{
				  buf[i]=(char)((num%10)+'0');
				  num/=10;
			}
		}
		
		i++;
		i++;
		buf2[i] = '\0';
		j = 0;
		i--;

		for(;i>=0;i--)
		{
			if(j==4)
			{
				buf2[i]='.';
				i--;
			}              
				buf2[i]=buf[j];
				j++;
		}

		buf2[0]='-';
		return &buf2[0];
	}
	else
	{
		if (num==0)
		{
			buf[0] = '0';
			buf[1] = '\0';
			return &buf[0];
		}

		for(i=0;i<12 && (num!=0 || i<5);i++)
		{
			if (num==0)
				  buf[i]='0';                 
			else
			{
				  buf[i]=(char)((num%10)+'0');
				  num/=10;
			}
		}
		
		i++;
		buf2[i] = '\0';
		j = 0;
		i--;
		for(;i>=0;i--)
		{
			if(j==4)
			{
				buf2[i]='.';
				i--;
			}              
			buf2[i]=buf[j];
			j++;
		}
		return &buf2[0];
	}
}

static
char *strcpy(char *dst, const char *src) {

	char *dstSave=dst;
   	int c;

   	do {
    	c = *dst++ = *src++;
   	} while(c);

   return dstSave;
}

static
int strlen(const char *string) {

   const char *base=string;

   while(*string++) ;

   return string - base - 1;
}

#endif
