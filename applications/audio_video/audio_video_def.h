/*
 * audio_video_def.h
 *
 *  Created on: 24/10/2016
 *      Author: mruaro
 */

#ifndef AUDIO_VIDEO_DEF_H_
#define AUDIO_VIDEO_DEF_H_

#define COMPRESSED_SAMPLES	(8*2)

#define FRAMES				20 //Memphis 4X4 sc demora 7s para cada frame

#define AUDIO_VIDEO_PERIOD	152000

#define IVLC_exe_time		70000
#define IVLC_deadline		AUDIO_VIDEO_PERIOD

#define FIR_exe_time		18000
#define FIR_deadline		AUDIO_VIDEO_PERIOD

#define IDCT_exe_time		9000
#define IDCT_deadline		AUDIO_VIDEO_PERIOD

#define IQUANT_exe_time		5000
#define IQUANT_deadline		AUDIO_VIDEO_PERIOD

#define JOIN_exe_time		2000
#define JOIN_deadline		AUDIO_VIDEO_PERIOD

#define SPLIT_exe_time		3000
#define SPLIT_deadline		AUDIO_VIDEO_PERIOD

#define ADPCM_DEC_exe_time	32000
#define ADPCM_DEC_deadline	AUDIO_VIDEO_PERIOD

#endif /* AUDIO_VIDEO_DEF_H_ */
