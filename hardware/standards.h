/*
 * Memphis_defaults.h
 *
 *  Created on: May 18, 2016
 *      Author: mruaro
 */

#ifndef STANDARDS_H_
#define STANDARDS_H_

#include <systemc.h>
#include <math.h>

#define EAST 	0
#define WEST 	1
#define NORTH 	2
#define SOUTH 	3
#define LOCAL 	4
#define NPORT 	5


#define TAM_FLIT 	32
#define METADEFLIT (TAM_FLIT/2)
#define QUARTOFLIT (TAM_FLIT/4)


// Memory map constants.
#define DEBUG 					0x20000000
#define IRQ_MASK 				0x20000010
#define IRQ_STATUS_ADDR 		0x20000020
#define TIME_SLICE_ADDR 		0x20000060
#define CLOCK_HOLD 				0x20000090
#define END_SIM 				0x20000080
#define NET_ADDRESS 			0x20000140

#define DMA_SIZE 				0x20000200
#define DMA_ADDR 				0x20000210
#define DMA_SIZE_2 				0x20000205
#define DMA_ADDR_2 				0x20000215
#define DMA_OP 					0x20000220
#define START_DMA 				0x20000230
#define DMA_ACK 				0x20000240
#define DMA_SEND_ACTIVE 		0x20000250
#define DMA_RECEIVE_ACTIVE		0x20000260

#define SCHEDULING_REPORT		0x20000270
#define ADD_PIPE_DEBUG			0x20000280
#define REM_PIPE_DEBUG			0x20000285
#define ADD_REQUEST_DEBUG		0x20000290
#define REM_REQUEST_DEBUG		0x20000295

#define TICK_COUNTER_ADDR 		0x20000300

#define SLACK_TIME_MONITOR		0x20000370

//Kernel pending service FIFO
#define PENDING_SERVICE_INTR	0x20000400

#define MEM_REG_PERIPHERALS 0x20000500

#define SLACK_MONITOR_WINDOW 	50000

//DMNI config code
#define CODE_CS_NET 		1
#define CODE_MEM_ADDR		2
#define CODE_MEM_SIZE  		3
#define CODE_MEM_ADDR2		4
#define CODE_MEM_SIZE2		5
#define CODE_OP				6

#define MEMORY_WORD_SIZE	4

#define BUFFER_TAM 			8 // must be power of two

typedef sc_uint<TAM_FLIT > regflit;
typedef sc_uint<16> regaddress;

typedef sc_uint<3> 				reg3;
typedef sc_uint<4> 				reg4;
typedef sc_uint<8> 				reg8;
typedef sc_uint<10> 			reg10;
typedef sc_uint<11> 			reg11;
typedef sc_uint<16> 			reg16;
typedef sc_uint<32> 			reg32;
typedef sc_uint<40> 			reg40;
typedef sc_uint<NPORT> 			regNport;
typedef sc_uint<TAM_FLIT> 		regflit;
typedef sc_uint<(TAM_FLIT/2)> 	regmetadeflit;
typedef sc_uint<(TAM_FLIT/4)> 	regquartoflit;
typedef sc_uint<(3*NPORT)> 		reg_mux;

#endif
