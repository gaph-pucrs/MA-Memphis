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


int main()
{
    // unsigned int time_a, time_b;
    int i;
    type_DATA block[64];

    puts("MPEG Task PRINT start:\n");

    // unsigned then = memphis_get_tick();

    memphis_real_time(RT_PERIOD, PRINT_DEADLINE, PRINT_EXEC_TIME);
    for(i=0;i<MPEG_FRAMES;i++)
    {
        memphis_receive(block, sizeof(block), idct);
        // unsigned now = memphis_get_tick();
		// printf("%u_%u\n", now, now - then);
		// then = now;
    }

    puts("End Task E - MPEG\n");

    return 0;
}
