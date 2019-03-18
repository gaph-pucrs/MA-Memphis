/* $Id: fir.c,v 1.4 2005/04/08 17:20:17 csg Exp $ */

/* Execute the integer fir filter from "C Algorithms for DSP".
 *
 * Adapted for WCET benchmarking by IJAE May 2000.
 *
 * Features: nested variable-length loops.
 *           if-statement with branches taken a known number of times
 *
 * Added explicit sizes of the arrays to work-around a bug in the
 * NIC-compiler. Christer Sandberg
 */

#include <api.h>
#include <stdlib.h>
#include "audio_video_def.h"
/*--------------------------------------------------
 *---- INPUT DATA FOR TESTING
 *--------------------------------------------------*/
int fir_int[36]={
0xfffffffe, 0x1, 0x4, 0x3, 0xfffffffe, 0xfffffffc, 0x2, 0x7, 0x0,
0xfffffff7, 0xfffffffc, 0xc, 0xb, 0xfffffff2, 0xffffffe6, 0xf, 0x59, 0x7f,
0x59, 0xf, 0xffffffe6, 0xfffffff2, 0xb, 0xc, 0xfffffffc, 0xfffffff7, 0x0,
0x7, 0x2, 0xfffffffc, 0xfffffffe, 0x3, 0x4, 0x1, 0xfffffffe, 0};


/**************************************************************************
fir_filter_int - Filters int data array based on passed int coefficients.

The length of the input and output arrays are equal
and are allocated by the calller.
The length of the coefficient array is passed.
An integer scale factor (passed) is used to divide the accumulation output_stream.

void fir_filter_int(int *in,int *out,int in_len,
                                  int *coef,int coef_len,int scale)

    in          integer pointer to input array
    out         integer pointer to output array
    in_len      length of input and output arrays
    coef        integer pointer to coefficient array
    coef_len    length of coeffient array
    scale       scale factor to divide after accumulation

No return value.

*************************************************************************/

void fir_filter_int(int* in,int* out,int in_len,
                    int* coef,int coef_len,
                    int scale)
{
  int i,j,coef_len2,acc_length;
  int acc;
  int *in_ptr,*data_ptr,*coef_start,*coef_ptr,*in_end;

  /* set up for coefficients */
  coef_start = coef;
  coef_len2 = (coef_len + 1) >> 1;

  /* set up input data pointers */
  in_end = in + in_len - 1;
  in_ptr = in + coef_len2 - 1;

  /* initial value of accumulation length for startup */
  acc_length = coef_len2;

  for(i = 0 ; i < in_len ; i++) {

    /* set up pointer for accumulation */
    data_ptr = in_ptr;
    coef_ptr = coef_start;

    /* do accumulation and write output_stream with scale factor */

    acc = (int)(*coef_ptr++) * (*data_ptr--);
    for(j = 1 ; j < acc_length ; j++)
      acc += (int)(*coef_ptr++) * (*data_ptr--);
    *out++ = (int)(acc/scale);

    /* check for end case */

    if(in_ptr == in_end) {
      acc_length--;       /* one shorter each time */
      coef_start++;       /* next coefficient each time */
    }

    /* if not at end, then check for startup, add to input pointer */

    else {
      if(acc_length < coef_len) acc_length++;
      in_ptr++;
    }
  }
}



int main () {
	int k;
	Message received_msg;
	Message send_msg;
	int * input_stream;

	Echo("FIR - start");

	//RealTime(AUDIO_VIDEO_PERIOD, FIR_deadline, FIR_exe_time);

	for(k=0; k<FRAMES; k++ ) {

		Receive(&received_msg, adpcm_dec);
		input_stream = received_msg.msg;

		/* Executes the filter over the input stream */
		fir_filter_int(input_stream, received_msg.msg, COMPRESSED_SAMPLES*2, fir_int, 35, 285);

		received_msg.length = COMPRESSED_SAMPLES;

		/* Sends the adpcm uncompressed stream */
		Send(&received_msg, join);

	}

	Echo("FIR - end");

	exit();
}
