#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "audio_video_def.h"
int main() {

    // unsigned char decoded_block[1000];
    // int samples[COMPRESSED_SAMPLES*2];
    // unsigned int j, time_arrive =0, last_arrive = 0, jitter[2000];
    // int block_size, blocks;
    message_t msg;
    int k;

    puts("Join start...\n");
    puts("Number of frames\n");
	//printf(itoa(FRAMES));

	//RealTime(AUDIO_VIDEO_PERIOD, JOIN_deadline, JOIN_exe_time);

	// j = 0;
	for(k=0; k<FRAMES; k++ ) {

		memphis_receive(&msg, FIR);

		memphis_receive(&msg,idct);

		//printf("%d", memphis_get_tick());

	}

    //for(i=0; i<j; i++)
    //	Echo(itoa(jitter[i]));

	puts("Join finished.\n");

	return 0;
}
