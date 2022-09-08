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
 * @brief Implements a DMA and NI module.
 */

#include "DMNI.hpp"

DMNI::DMNI(sc_module_name name_, regmetadeflit address_router_) :
	sc_module(name_), 
	address_router(address_router_)
{

	SC_METHOD(arbiter);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(config);
	sensitive << clock.pos();

	SC_METHOD(receive);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(send);
	sensitive << clock.pos();
	sensitive << reset;

	SC_METHOD(buffer_control);
	sensitive << add_buffer;
	sensitive << first;
	sensitive << last;

	SC_METHOD(credit_o_update);
	sensitive << slot_available;

	SC_METHOD(mem_address_update);
	sensitive << read_enable;
	sensitive << write_enable;
	sensitive << recv_address;
	sensitive << send_address;
	sensitive << br_rcv_enable;
	sensitive << br_mem_addr;
	sensitive << br_mem_data;
	sensitive << br_byte_we;
	sensitive << noc_byte_we;
	sensitive << noc_data_write;
	sensitive << br_payload;

	SC_METHOD(br_receive);
	sensitive << clock.pos();
	sensitive << reset;
}

void DMNI::arbiter()
{
	if(reset.read() == 1){
		write_enable = false;
		read_enable = false;
		br_rcv_enable = false;
		last_arb = SEND;
		timer = 0;

		ARB = ROUND;
		return;
	}

	switch(ARB){
		case ROUND:
		{
			switch(last_arb){
				case SEND:
				{
					if(DMNI_Receive == IDLE){
						ARB = RECEIVE;
						write_enable = true;
					} else if(br_req_mon && !br_ack_mon && monitor_ptrs[br_mon_svc] != 0){
						ARB = BR_RECEIVE;
						br_rcv_enable = true;
					} else if(send_active){
						ARB = SEND;
						read_enable = true;
					}
					break;
				}
				case RECEIVE:
				{
					if(br_req_mon && !br_ack_mon && monitor_ptrs[br_mon_svc] != 0){
						ARB = BR_RECEIVE;
						br_rcv_enable = true;
					} else if(send_active){
						ARB = SEND;
						read_enable = true;
					} else if(DMNI_Receive == IDLE){
						ARB = RECEIVE;
						write_enable = true;
					}
					break;
				}
				case BR_RECEIVE:
				{
					if(send_active){
						ARB = SEND;
						read_enable = true;
					} else if(DMNI_Receive == IDLE){
						ARB = RECEIVE;
						write_enable = true;
					} else if(br_req_mon && !br_ack_mon && monitor_ptrs[br_mon_svc] != 0){
						ARB = BR_RECEIVE;
						br_rcv_enable = true;
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case SEND:
		{
			if(DMNI_Send == END || (timer >= DMNI_TIMER && receive_active)){
				timer = 0;
				ARB = ROUND;
				last_arb = SEND;
				read_enable = false;
			} else {
				timer = timer + 1;
			}
			break;
		}
		case RECEIVE:
		{
			if(DMNI_Receive == END || (timer >= DMNI_TIMER && send_active)){
				timer = 0;
				ARB = ROUND;
				last_arb = RECEIVE;
				write_enable = false;
			} else {
				timer = timer + 1;
			}
			break;
		}
		case BR_RECEIVE:
		{
			if(br_rcv_state == BR_RCV_END){
				ARB = ROUND;
				last_arb = BR_RECEIVE;
				br_rcv_enable = false;
			}
			break;
		}
	}
}

void DMNI::config()
{
	if(reset){
		for(int i = 0; i < MON_TABLE_MAX; i++)
			monitor_ptrs[i] = 0;

		return;
	}

	if(set_address){
		address = config_data;
		address_2 = 0;
	} else if(set_address_2){
		address_2 = config_data;
	} else if(set_size){
		size = config_data;
		size_2 = 0;
	} else if(set_size_2){
		size_2 = config_data;
	} else if(set_op){
		operation = config_data.read();
	} else if(set_mon_qos){
		monitor_ptrs[MON_TABLE_QOS] = config_data.read();
	} else if(set_mon_pwr){
		monitor_ptrs[MON_TABLE_PWR] = config_data.read();
	} else if(set_mon_2){
		monitor_ptrs[MON_TABLE_2] = config_data.read();
	} else if(set_mon_3){
		monitor_ptrs[MON_TABLE_3] = config_data.read();
	} else if(set_mon_4){
		monitor_ptrs[MON_TABLE_4] = config_data.read();
	}
}

void DMNI::mem_address_update()
{
	if (read_enable.read() == 1){
		mem_address.write(send_address.read());
	} else if(write_enable){
		mem_address.write(recv_address.read());
		mem_data_write.write(noc_data_write.read());
		mem_byte_we.write(noc_byte_we.read());
	} else if(br_rcv_enable){
		mem_address.write(br_mem_addr.read());
		mem_data_write.write(br_mem_data.read());
		mem_byte_we.write(br_byte_we.read());
	} else {
		/* Avoid writing when no operation is occurring */
		mem_byte_we.write(0);
	}
}

void DMNI::credit_o_update() {
	credit_o.write(slot_available.read());
}

void DMNI::buffer_control(){

	//Buffer full
	if ( ( first.read() == last.read() ) && add_buffer.read() == 1){
		slot_available.write(0);
	} else {
		slot_available.write(1);
	}

	//Buffer empty
	if ( ( first.read() == last.read() ) && add_buffer.read() == 0){
		read_av.write(0);
	} else {
		read_av.write(1);
	}
}

void DMNI::receive()
{
	if(reset){
		first.write(0);
		last.write(0);
		payload_size.write(0);
		SR.write(HEADER);
		add_buffer.write(0);
		receive_active.write(0);
		DMNI_Receive.write(WAIT);
		intr_count.write(0);
		for(int i=0; i<BUFFER_SIZE; i++){ //in vhdl replace by OTHERS=>'0'
			is_header[i] = 0;
		}
		return;
	}

	sc_uint<4> intr_counter_temp = intr_count.read();

	//Read from NoC
	if (rx.read() == 1 && slot_available.read() == 1){

		buffer[last.read()].write(data_in.read());
		add_buffer.write(1);
		last.write(last.read() + 1);

		switch (SR.read()) {
			case HEADER:
				intr_counter_temp = intr_counter_temp + 1;
				/*if(address_router == 0){
					cout<<"Master receiving msg "<<endl;
				}*/
				is_header[last.read()] = 1;
				SR.write(PAYLOAD);
			break;

			case PAYLOAD:
				is_header[last.read()] = 0;
				payload_size.write(data_in.read() - 1);
				SR.write(DATA);
			break;

			case DATA:
				is_header[last.read()] = 0;
				if (payload_size.read() == 0){
					SR.write(HEADER);
				} else {
					payload_size.write(payload_size.read() - 1);
				}

			break;
		}
	}

	// if(noc_byte_we.read() == 0xF)
		// std::cout << "Copy " << std::hex << noc_data_write.read() << " to " << recv_address.read() << std::endl;

	//Write to memory
	switch (DMNI_Receive.read()) {
		case WAIT:
			if(start.read() == 1 && operation.read() == 1){
				recv_address.write(address.read() - WORD_SIZE);
				recv_size.write(size.read());

				if (is_header[first.read()] == 1 && intr_counter_temp > 0){
					intr_counter_temp = intr_counter_temp - 1;
				}
				receive_active.write(1);

				DMNI_Receive.write(WAIT_DATA);
			}
			break;
		case WAIT_DATA:
			noc_byte_we.write(0);
			if(read_av.read()){
				noc_data_write.write(buffer[first.read()].read());
				recv_address.write(recv_address.read() + WORD_SIZE);

				add_buffer.write(0);
				first.write(first.read() + 1);
				recv_size.write(recv_size.read() - 1);

				// if(write_enable.read()){
				// 	noc_byte_we.write(0xF);
				// 	if(recv_size.read() == 1)
				// 		DMNI_Receive.write(END);
				// } else {
					DMNI_Receive.write(IDLE);
				// }
			}
			break;
		case IDLE:
			if(write_enable.read()){
				noc_byte_we.write(0xF);
				if(recv_size.read() == 0){
					DMNI_Receive.write(END);
				} else {
					/**
					 * @todo
					 * Optimize here to receive next flit
					 */
					DMNI_Receive.write(WAIT_DATA);
				}
			}
			break;
		case END:
			receive_active.write(0);
			noc_byte_we.write(0);
			recv_address.write(0);
			recv_size.write(0);
			DMNI_Receive.write(WAIT);
			break;
		default:
			break;
	}

	//Interruption management
	if (intr_counter_temp > 0){
		intr.write(1);
	} else {
		intr.write(0);
	}
	intr_count.write(intr_counter_temp);
}

void DMNI::send()
{
	if(reset){
		DMNI_Send.write(WAIT);
		send_active.write(0);
		tx.write(0);
		return;
	}

	switch (DMNI_Send.read()) {
		case WAIT:
			if (start.read() == 1 && operation.read() == 0){
				send_address.write(address.read());
				send_address_2.write(address_2.read());
				send_size.write(size.read());
				send_size_2.write(size_2.read());
				send_active.write(1);
				DMNI_Send.write(LOAD);
				/*if(address_router == 0){
					cout<<"Master sending msg "<<endl;
				}*/
			}
		break;

		case LOAD:

			if (credit_i.read() == 1 && read_enable.read() == 1){
				send_address.write(send_address.read() + WORD_SIZE);
				DMNI_Send.write(COPY_FROM_MEM);
			}
		break;

		case COPY_FROM_MEM:

			if (credit_i.read() == 1 && read_enable.read() == 1){

				if (send_size.read() > 0){

					tx.write(1);
					data_out.write(mem_data_read.read());
					send_address.write(send_address.read() + WORD_SIZE);
					send_size.write(send_size.read() - 1);

				} else if (send_size_2.read() > 0) {

					send_size.write(send_size_2.read());
					send_size_2.write(0);
					tx.write(0);
					if (send_address_2.read()(30,28) == 0){
						send_address.write(send_address_2.read());
					} else {
						send_address.write(send_address_2.read() - WORD_SIZE);
					}
					DMNI_Send.write(LOAD);

				} else {
					tx.write(0);
					DMNI_Send.write(END);
				}
			} else {

				if (credit_i.read() == 0){
					send_size.write(send_size.read() + 1);
					send_address.write(send_address.read() - (WORD_SIZE + WORD_SIZE)); // endereco volta 2 posicoes
				} else {
					send_address.write(send_address.read() - WORD_SIZE);  // endereco volta 1 posicoes
				}
				tx.write(0);
				DMNI_Send.write(LOAD);
			}

		break;

		case END:
			send_active.write(0);
			send_address.write(0);
			send_address_2.write(0);
			send_size.write(0);
			send_size_2.write(0);
			DMNI_Send.write(WAIT);
		break;

		default:
			break;
	}
}

void DMNI::br_receive()
{
	if(reset){
		br_byte_we = 0;
		br_ack_mon = false;
		br_rcv_state = BR_RCV_IDLE;
		for(int n = 0; n < N_PE; n++){
			for(int t = 0; t < TASK_PER_PE; t++){
				mon_table[n][t] = -1;
			}
		}

		return;
	}

	if(!br_req_mon)
		br_ack_mon = false;
	else if(monitor_ptrs[br_mon_svc] == 0)	/* Ignore invalid requests */
		br_ack_mon = true;

	switch(br_rcv_state){
		case BR_RCV_IDLE:
		{
			if(br_rcv_enable){
				br_mem_data = br_payload;

				uint32_t ptr = monitor_ptrs[br_mon_svc];
				uint16_t src = br_address >> 16;
				uint16_t seq_addr = (src >> 8) + (src & 0xFF)*N_PE_X;
				ptr += (seq_addr * TASK_PER_PE * 8);

				uint16_t task = br_producer;

				int idx = -1;
				for(int i = 0; i < TASK_PER_PE; i++){
					if(mon_table[seq_addr][i] == task){
						idx = i;
						break;
					}
				}

				if(idx != -1){
					ptr += (idx * 8 + 4);
					br_mem_addr = ptr;

					br_byte_we = 0xF;
					br_rcv_state = BR_RCV_TASKID;
				} else {
					for(int i = 0; i < TASK_PER_PE; i++){
						if(mon_table[seq_addr][i] == -1){
							idx = i;
							break;
						}
					}

					if(idx != -1){
						mon_table[seq_addr][idx] = task;
						ptr += (idx * 8 + 4);
						br_mem_addr = ptr;

						br_byte_we = 0xF;
						br_rcv_state = BR_RCV_TASKID;
					} else {
						cout << "ERROR: NO AVAILABLE SPACE IN MONITOR LUT -- NEED CLEANUP" << endl;
						br_rcv_state = BR_RCV_END;
						br_ack_mon = true;
					}
				}
			}
			break;
		}
		case BR_RCV_TASKID:
		{
			br_mem_data = br_producer;

			uint32_t ptr = br_mem_addr - 4;
			br_byte_we = 0xF;

			br_mem_addr = ptr;
			br_ack_mon = true;
			br_rcv_state = BR_RCV_END;
			break;
		}
		case BR_RCV_END:
		{
			br_byte_we = 0;
			br_rcv_state = BR_RCV_IDLE;
			break;
		}
	}

	/**
	 * @todo
	 * Optimize this with 64 bit payload (add source PE to addressing)
	 */
	if(clear_task){
		int16_t task_to_clear = config_data.read();
		bool found = false;
		for(int n = 0; n < N_PE; n++){
			for(int t = 0; t < TASK_PER_PE; t++){
				if(mon_table[n][t] == task_to_clear){
					mon_table[n][t] = -1;
					found = true;
					// cout << "Cleared task " << task_to_clear << " from table PE " << n << " id " << t << endl;
					break;
				}
			}
			if(found)
				break;
		}
	}
}
