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

#include <api.h>
#include <stdlib.h>
#include "mpeg_std.h"

typedef int type_DATA; //unsigned

Message msg1;

int main()
{
    unsigned int time_a, time_b;
    int i;

    Echo("MPEG Task PRINT start:");

    for(i=0;i<MPEG_FRAMES;i++)
    {
        Receive(&msg1,idct);
        Echo(itoa(GetTick()));
    }

    Echo("End Task E - MPEG");

    exit();
}
