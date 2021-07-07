/*---------------------------------------------------------------------
TITLE: Program Scheduler
AUTHOR: Nicolas Saint-jean
EMAIL : saintjea@lirmm.fr
DATE CREATED: 04/04/06
FILENAME: task3.c
PROJECT: Network Process Unit
COPYRIGHT: Software placed into the public domain by the author.
           Software 'as is' without warranty.  Author liable for nothing.
DESCRIPTION: This file contains the task3
---------------------------------------------------------------------*/

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpeg_std.h"

typedef int type_DATA; //unsigned

message_t msg1;

int main()
{
    // unsigned int time_a, time_b;
    int i;

    puts("MPEG Task PRINT start:\n");

    for(i=0;i<MPEG_FRAMES;i++)
    {
        memphis_receive(&msg1,idct);
        //printf("%d\n", memphis_get_tick());
    }

    puts("End Task E - MPEG\n");

    return 0;
}
