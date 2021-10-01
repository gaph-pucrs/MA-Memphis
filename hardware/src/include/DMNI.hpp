/**
 * MA-Memphis
 * @file DMNI.hpp
 * 
 * @author Unknown
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2013
 * 
 * @brief Implements a DMNI used by NI module.
 */

#pragma once

#include <systemc>

#include "standards.h"

/*
 * send - read from memory and write to noc
 * recv - read from noc and write to memory
 */

SC_MODULE(DMNI){
public:
	sc_in<bool>				clock;
	sc_in<bool>				reset;

	/* Memory-mapped configuration interface */
	sc_in<bool>				set_address;
	sc_in<bool>				set_address_2;
	sc_in<bool>				set_size;
	sc_in<bool>				set_size_2;
	sc_in<bool>				set_op;
	sc_in<bool>				set_mon_qos;
	sc_in<bool>				set_mon_pwr;
	sc_in<bool>				set_mon_2;
	sc_in<bool>				set_mon_3;
	sc_in<bool>				set_mon_4;
	sc_in<bool>				start;
	sc_in<bool>				clear_task;
	sc_in<sc_uint<32>>		config_data;

	/* Output Status */
	sc_out<bool>			intr;
	sc_out<bool>			send_active;
	sc_out<bool>			receive_active;

	/* Internal memory interface */
	sc_out<sc_uint<32>>		mem_address;
	sc_out<sc_uint<32>>		mem_data_write;
	sc_in<sc_uint<32>>		mem_data_read;
	sc_out<sc_uint<4>>		mem_byte_we;

	/* NoC Interface (Local port) */
	sc_out<bool> 			tx;
	sc_out<regflit> 		data_out;
	sc_in<bool> 			credit_i;
	sc_in<bool> 			rx;
	sc_in<regflit>			data_in;
	sc_out<bool> 			credit_o;

	/* BrNoC Interface (Local port output) */
	sc_in<bool> 			br_req_mon;
	sc_out<bool>			br_ack_mon;
	sc_in<uint8_t>			br_mon_svc;
	sc_in<uint16_t>			br_producer;
	sc_in<uint32_t>			br_address;
	sc_in<uint32_t>			br_payload;

	SC_HAS_PROCESS(DMNI);
	DMNI(sc_module_name name_, regmetadeflit address_router_ = 0);

private:
	static const uint8_t BUFFER_SIZE = 16; /* Changing size implies changing the number of bits of first and last */
	static const uint8_t DMNI_TIMER = 16;
	static const uint8_t WORD_SIZE = 4;
	static const uint8_t TASK_PER_PE = (MEMORY_SIZE_BYTES-PAGE_SIZE_BYTES)/PAGE_SIZE_BYTES;

	enum dmni_state {
		WAIT, 
		LOAD, 
		COPY_FROM_MEM, 
		COPY_TO_MEM, 
		END
	};

	enum state_noc {
		HEADER, 
		PAYLOAD, 
		DATA
	};

	enum arbiter_state {
		ROUND, 
		SEND, 
		RECEIVE,
		BR_RECEIVE
	};

	enum BR_RCV_STATE {
		BR_RCV_IDLE, 
		BR_RCV_TASKID, 
		BR_RCV_END
	};

	regmetadeflit address_router;

	sc_signal<dmni_state>	DMNI_Send, DMNI_Receive;
	sc_signal<state_noc>	SR;
	sc_signal<BR_RCV_STATE>	br_rcv_state;
	arbiter_state			ARB;

	sc_signal<sc_uint<32>>	buffer[BUFFER_SIZE];
	sc_signal<bool >		is_header[BUFFER_SIZE];
	sc_signal<sc_uint<4>>	intr_count;

	sc_signal<sc_uint<4>>	first, last;
	sc_signal<bool>			add_buffer;

	sc_signal<regflit> 		payload_size;

	sc_signal<sc_uint<32>> 	address;
	sc_signal<sc_uint<32>> 	address_2;
	sc_signal<sc_uint<32>> 	size;
	sc_signal<sc_uint<32>> 	size_2;
	sc_signal<sc_uint<32>>	send_address;
	sc_signal<sc_uint<32>>	send_address_2;
	sc_signal<sc_uint<32>>	send_size;
	sc_signal<sc_uint<32>>	send_size_2;
	sc_signal<sc_uint<32>>	recv_address;
	sc_signal<sc_uint<32>>	recv_size;
	sc_signal<bool>			operation;
	sc_signal<bool>			read_av;
	sc_signal<bool>			slot_available;
	sc_signal<bool>			write_enable;
	sc_signal<bool>			read_enable;

	sc_signal<uint32_t>		monitor_ptrs[MON_TABLE_MAX];
	sc_signal<bool>			br_rcv_enable;
	sc_signal<uint32_t>		br_mem_addr;
	sc_signal<uint32_t>		br_mem_data;
	sc_signal<uint8_t>		br_byte_we;

	sc_signal<uint8_t>		noc_byte_we;
	sc_signal<uint32_t>		noc_data_write;

	arbiter_state			last_arb;
	uint8_t					timer;
	int16_t					mon_table[N_PE][TASK_PER_PE];

	void config();
	void receive();
	void send();
	void buffer_control();
	void arbiter();
	void credit_o_update();
	void mem_address_update();
	void br_receive();
};
