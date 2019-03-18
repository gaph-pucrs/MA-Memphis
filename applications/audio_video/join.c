#include <api.h>
#include <stdlib.h>
#include "audio_video_def.h"
int main() {

    unsigned char decoded_block[1000];
    int samples[COMPRESSED_SAMPLES*2];
    unsigned int j, time_arrive =0, last_arrive = 0, jitter[2000];
    int block_size, blocks;
    Message msg;
    int i, k;

    Echo("Join start...");
    Echo("Number of frames");
	Echo(itoa(FRAMES));

	//RealTime(AUDIO_VIDEO_PERIOD, JOIN_deadline, JOIN_exe_time);

	j = 0;
	for(k=0; k<FRAMES; k++ ) {

		Receive(&msg, FIR);

		Receive(&msg,idct);

		Echo(itoa(GetTick()));

	}

    //for(i=0; i<j; i++)
    //	Echo(itoa(jitter[i]));

	Echo("Join finished.");

	exit();
}
