/*
 * syn_std.h
 *
 *  Created on: 09/07/2016
 *      Author: ruaro
 */

#ifndef SYN_STD_H_
#define SYN_STD_H_


#define SYNTHETIC_ITERATIONS	100

void compute(unsigned int * data){
	volatile int t;
	for(t=0; t<1000; t++);
	for (int i=0; i<128; i++){
		data[i] = i;
	}
}


#endif /* SYN_STD_H_ */
