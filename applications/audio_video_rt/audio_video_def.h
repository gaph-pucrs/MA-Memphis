/*
 * audio_video_def.h
 *
 *  Created on: 24/10/2016
 *      Author: mruaro
 */

#ifndef AUDIO_VIDEO_DEF_H_
#define AUDIO_VIDEO_DEF_H_

#include "config.h"

#define COMPRESSED_SAMPLES	(8*2)

#define FRAMES				20 //Memphis 4X4 sc demora 7s para cada frame

#define AUDIO_VIDEO_PERIOD	RT_PERIOD

#define IVLC_exe_time		 81000
#define IVLC_deadline		AUDIO_VIDEO_PERIOD

#define FIR_exe_time		 25000
#define FIR_deadline		AUDIO_VIDEO_PERIOD

#define IDCT_exe_time		 11000
#define IDCT_deadline		AUDIO_VIDEO_PERIOD

#define IQUANT_exe_time		  9000
#define IQUANT_deadline		AUDIO_VIDEO_PERIOD

#define ADPCM_DEC_exe_time	 56000
#define ADPCM_DEC_deadline	AUDIO_VIDEO_PERIOD

#endif /* AUDIO_VIDEO_DEF_H_ */
