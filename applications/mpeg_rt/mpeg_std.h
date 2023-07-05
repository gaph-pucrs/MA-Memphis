/*
 * package.h
 *
 *  Created on: 09/07/2016
 *      Author: ruaro
 */

#ifndef MPEG_STD_H_
#define MPEG_STD_H_

#ifndef MPEG_FRAMES
	#define MPEG_FRAMES				       20
#endif

#ifndef RT_PERIOD
	#define RT_PERIOD				   120000
#endif

#ifndef START_EXEC_TIME
	#define START_EXEC_TIME			      500
#endif

#ifndef START_DEADLINE
	#define START_DEADLINE			RT_PERIOD
#endif

#ifndef IVLC_EXEC_TIME
	#define IVLC_EXEC_TIME			    81000
#endif

#ifndef IVLC_DEADLINE
	#define IVLC_DEADLINE			RT_PERIOD
#endif

#ifndef IQUANT_EXEC_TIME
	#define IQUANT_EXEC_TIME		     8300
#endif

#ifndef IQUANT_DEADLINE
	#define IQUANT_DEADLINE			RT_PERIOD
#endif

#ifndef IDCT_PERIOD
	#define IDCT_PERIOD				RT_PERIOD
#endif

#ifndef IDCT_EXEC_TIME
	#define IDCT_EXEC_TIME			    10500
#endif

#ifndef IDCT_DEADLINE
	#define IDCT_DEADLINE			RT_PERIOD
#endif

#ifndef PRINT_EXEC_TIME
	#define PRINT_EXEC_TIME			      500
#endif

#ifndef PRINT_DEADLINE
	#define PRINT_DEADLINE			RT_PERIOD
#endif

#endif /* PACKAGE_H_ */
