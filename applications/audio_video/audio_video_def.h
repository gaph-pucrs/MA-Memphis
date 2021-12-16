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


#define IVLC_exe_time		36000
#define IVLC_deadline		76000

#define FIR_exe_time		13000
#define FIR_deadline		76000

#define IDCT_exe_time		6000
#define IDCT_deadline		76000

#define IQUANT_exe_time		4500
#define IQUANT_deadline		76000

#define JOIN_exe_time		1800
#define JOIN_deadline		76000

#define SPLIT_exe_time		2400
#define SPLIT_deadline		76000

#define ADPCM_DEC_exe_time	23000
#define ADPCM_DEC_deadline	76000

#define AUDIO_VIDEO_PERIOD	76000


#endif /* AUDIO_VIDEO_DEF_H_ */
