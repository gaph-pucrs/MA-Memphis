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
    type_DATA block[64];

    // unsigned then = memphis_get_tick();
    for(int i = 0; i < MPEG_FRAMES; i++)
    {
        // unsigned now = memphis_get_tick();
		// printf("T: %d\n", now - then);
        memphis_receive(block, sizeof(block), idct);
        // then = memphis_get_tick();
    }

    return 0;
}
