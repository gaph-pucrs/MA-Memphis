/**
 * MA-Memphis
 * @file PE.hpp
 * 
 * @author Unknown
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2013
 * 
 * @brief Join all PE modules. Manipulate memory mapped registers
 */

#pragma once

#include <systemc>

#include "mlite_cpu.h"
#include "DMNI.hpp"
#include "router_cc.h"
#include "ram.h"
#include "BrLiteRouter.hpp"
#include "BrLiteBuffer.hpp"
#include "BrLiteControl.hpp"

#include "standards.h"

SC_MODULE(PE) {
	
	sc_in<bool>			clock;
	sc_in<bool>			reset;

	/* Hermes NoC interface */
	sc_out<bool>		tx[NPORT-1];
	sc_out<regflit>		data_out[NPORT-1];
	sc_in<bool>			credit_i[NPORT-1];
	
	sc_in<bool> 		rx[NPORT-1];
	sc_in<regflit>		data_in[NPORT-1];
	sc_out<bool>		credit_o[NPORT-1];

	sc_in<bool>			br_req_in[NPORT - 1];
	sc_in<bool>			br_ack_in[NPORT - 1];
	sc_in<uint32_t>		br_payload_in[NPORT - 1];
	sc_in<uint32_t>		br_address_in[NPORT - 1];
	sc_in<uint16_t>		br_producer_in[NPORT - 1];
	sc_in<uint8_t>		br_id_svc_in[NPORT - 1];
	sc_in<uint8_t>		br_ksvc_in[NPORT - 1];

	sc_out<bool>		br_req_out[NPORT - 1];
	sc_out<bool>		br_ack_out[NPORT - 1];
	sc_out<uint32_t>	br_payload_out[NPORT - 1];
	sc_out<uint32_t>	br_address_out[NPORT - 1];
	sc_out<uint16_t>	br_producer_out[NPORT - 1];
	sc_out<uint8_t>		br_id_svc_out[NPORT - 1];
	sc_out<uint8_t>		br_ksvc_out[NPORT - 1];

	mlite_cpu	*	cpu;
	ram			* 	mem;
	DMNI 			dmni;
	router_cc 	*	router;
	BrLiteRouter	br_router;
	BrLiteBuffer	br_buffer;
	BrLiteControl	br_control;
	
	SC_HAS_PROCESS(PE);
	PE(sc_module_name name_, regaddress address_ = 0x00, std::string path_ = "");
	
private:

	sc_signal<bool> 		clock_hold;
  	sc_signal<bool>  		credit_signal[NPORT - 1];
	bool 					clock_aux;

	/* Signals */
	sc_signal<sc_uint<32>> 	cpu_mem_address_reg;
	sc_signal<sc_uint<32>> 	cpu_mem_data_write_reg;
	sc_signal<sc_uint<4>> 	cpu_mem_write_byte_enable_reg;
	sc_signal<sc_uint<8>> 	irq_mask_reg;
	sc_signal<sc_uint<8>> 	irq_status;
	sc_signal<bool> 		irq;
	sc_signal<sc_uint<32>> 	time_slice;
	sc_signal<bool> 		write_enable;
	sc_signal<sc_uint<32>>	tick_counter_local;
	sc_signal<sc_uint<32>>	tick_counter;
	sc_signal<sc_uint<8>>	current_page;

	/* CPU */
	sc_signal<sc_uint<32>>	cpu_mem_address;
	sc_signal<sc_uint<32>>	cpu_mem_data_write;
	sc_signal<sc_uint<32>>	cpu_mem_data_read;
	sc_signal<sc_uint<4>> 	cpu_mem_write_byte_enable;
	sc_signal<bool> 		cpu_mem_pause;
	sc_signal<bool> 		cpu_enable_ram;
	sc_signal<bool> 		cpu_set_size;
	sc_signal<bool> 		cpu_set_address;
	sc_signal<bool> 		cpu_set_size_2;
	sc_signal<bool> 		cpu_set_address_2;
	sc_signal<bool> 		cpu_set_op;
	sc_signal<bool> 		cpu_start;
	sc_signal<bool> 		cpu_ack;
	sc_signal<bool> 		cpu_set_mon_qos;
	sc_signal<bool> 		cpu_set_mon_pwr;
	sc_signal<bool> 		cpu_set_mon_2;
	sc_signal<bool> 		cpu_set_mon_3;
	sc_signal<bool> 		cpu_set_mon_4;

	/* RAM */
	sc_signal<sc_uint<32>> 	data_read_ram;
	sc_signal<sc_uint<32>> 	mem_data_read;

	/* Network Interface */
	sc_signal<bool> 		ni_intr;

	/* Hermes NoC */
	sc_signal<bool> 		tx_ni;
	sc_signal<regflit> 		data_out_ni;
	sc_signal<bool> 		credit_i_ni;
	sc_signal<bool> 		rx_ni;
	sc_signal<regflit> 		data_in_ni;
	sc_signal<bool> 		credit_o_ni;

	/* DMNI */
	sc_signal<sc_uint<32>> 	dmni_mem_address;
	sc_signal<sc_uint<32>> 	dmni_mem_addr_ddr;
	sc_signal<bool>			dmni_mem_ddr_read_req;
	sc_signal<bool>			mem_ddr_access;
	sc_signal<sc_uint<4>> 	dmni_mem_write_byte_enable;
	sc_signal<sc_uint<32>>	dmni_mem_data_write;
	sc_signal<sc_uint<32>>	dmni_mem_data_read;
	sc_signal<sc_uint<32>>	dmni_data_read;
	sc_signal<bool> 		dmni_enable_internal_ram;
	sc_signal<bool> 		dmni_send_active_sig;
	sc_signal<bool> 		dmni_receive_active_sig;
	sc_signal<sc_uint<30>> 	address_mux;
	sc_signal<sc_uint<32>> 	cpu_mem_address_reg2;
	sc_signal<sc_uint<30>> 	addr_a;
	sc_signal<sc_uint<30>> 	addr_b;

	/* Pending service signal */
	sc_signal<bool>			pending_service;

	/* Router signals */
	/* Not reset for router */
	sc_signal<bool> 		reset_n;

	/**
	 * @DEPRECATED: END SIM 
	 */
	sc_signal<sc_uint<32>> 	end_sim_reg;

	sc_signal<sc_uint<32>> 	slack_update_timer;

	/* Signal to release Peripherals */
	sc_signal<regflit> 		mem_peripheral;

	/* BrNoC Signals */
	/* Local -> Network */
	sc_signal<bool>			br_req_in_local;
	sc_signal<bool>			br_ack_out_local;
	sc_signal<uint32_t>		br_payload_in_local;
	sc_signal<uint32_t>		br_address_in_local;
	sc_signal<uint16_t>		br_producer_in_local;
	sc_signal<uint8_t>		br_id_svc_in_local;
	sc_signal<uint8_t>		br_ksvc_in_local;
	sc_signal<bool>			br_local_busy;

	/* Network -> Local */
	sc_signal<bool>			br_req_out_local;
	sc_signal<bool>			br_ack_in_local;
	sc_signal<uint32_t>		br_payload_out_local;
	sc_signal<uint32_t>		br_address_out_local;
	sc_signal<uint16_t>		br_producer_out_local;
	sc_signal<uint8_t>		br_id_svc_out_local;
	sc_signal<uint8_t>		br_ksvc_out_local;

	/* BrNoC Output buffer for kernel messages (TGT and ALL) */
	sc_signal<bool>			br_buf_req;
	sc_signal<bool>			br_buf_ack;
	sc_signal<bool>			br_buf_empty;
	sc_signal<bool>			br_buf_read_in;
	sc_signal<bool>			br_buf_pop;
	sc_signal<bool>			br_dmni_clear;
	sc_signal<uint32_t>		br_buf_payload_out;
	sc_signal<uint32_t>		br_buf_producer_out;
	sc_signal<uint8_t>		br_buf_ksvc_out;

	/* BrNoC Control for sending messages through it */
	sc_signal<bool>			br_cfg_payload;
	sc_signal<bool>			br_cfg_address;
	sc_signal<bool>			br_cfg_producer;
	sc_signal<bool>			br_cfg_id_svc;
	sc_signal<bool>			br_cfg_ksvc;
	sc_signal<bool>			br_cfg_start;
	sc_signal<uint32_t>		br_cfg_data;

	sc_signal<uint32_t>		br_cfg_payload_out;
	sc_signal<uint32_t>		br_cfg_address_out;
	sc_signal<uint16_t>		br_cfg_producer_out;
	sc_signal<uint8_t>		br_cfg_id_svc_out;
	sc_signal<uint8_t>		br_cfg_ksvc_out;
	sc_signal<bool>			br_cfg_req_out;
	sc_signal<bool>			br_cfg_ack_in;

	/* BrNoC Monitoring via DMNI */
	sc_signal<bool>			br_dmni_ack;
	sc_signal<bool>			br_dmni_req;
	sc_signal<uint8_t>		br_dmni_svc;
	sc_signal<uint16_t>		br_dmni_prod;
	sc_signal<uint32_t>		br_dmni_addr;

	unsigned char shift_mem_page;

	unsigned long int log_interaction;
	unsigned long int instant_instructions;
	unsigned long int aux_instant_instructions;
	
	unsigned long int logical_instant_instructions;
	unsigned long int jump_instant_instructions;
	unsigned long int branch_instant_instructions;
	unsigned long int move_instant_instructions;
	unsigned long int other_instant_instructions;
	unsigned long int arith_instant_instructions;
	unsigned long int load_instant_instructions;
	unsigned long int shift_instant_instructions;
	unsigned long int nop_instant_instructions;
	unsigned long int mult_div_instant_instructions;

	char x_address;
	char y_address;

	char aux[255];
	FILE *fp;

	//logfilegen *log;

	regaddress router_address;
	std::string path;

	void sequential_attr();
	void log_process();
	void comb_assignments();
	void mem_mapped_registers();
	void reset_n_attr();
	void clock_stop();
	void end_of_simulation();
	void update_credit();
	void br_local_ack();
};
