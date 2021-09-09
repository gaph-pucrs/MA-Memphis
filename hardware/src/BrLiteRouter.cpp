/**
 * MA-Memphis
 * @file BrLiteRouter.cpp
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2021
 * 
 * @brief Light BrNoC router module -- Removed backtrack (unicast)
 */

#include "BrLiteRouter.hpp"

#include <iostream>

BrLiteRouter::BrLiteRouter(sc_module_name _name, uint16_t _address) :
	sc_module(_name),
	router_address(_address)
{
	SC_METHOD(input);
	sensitive << clock.pos();

	SC_METHOD(output);
	sensitive << clock.pos();

	SC_METHOD(input_output);
	sensitive << clock.pos();
}

void BrLiteRouter::input()
{
	if(reset){
		for(int i = 0; i < NPORT; i++)
			ack_out[i] = false;
		
		in_state = IN_INIT;
		selected_port = LOCAL;
		source_idx = 0;
		return;
	}

	switch(in_state){
		case IN_INIT:
		{
			if(!clear_local){
				bool has_request = false;
				for(int i = 0; i < NPORT; i++)
					has_request |= req_in[i];
				
				if(has_request){
					in_state = IN_ARBITRATION;
					// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": has req" << std::endl;
				}
			}
			break;
		}
		case IN_ARBITRATION:
		{
			uint8_t port = (selected_port + 1) % NPORT;
			while(port != selected_port){
				if(req_in[port]){
					selected_port = port;
					break;
				}
				port++;
				port %= NPORT;
			}
			// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": arbitred port " << (int)port << std::endl;

			in_state = IN_TEST_SPACE;
			break;
		}
		case IN_TEST_SPACE:
		{
			bool is_in_table = false;
			uint16_t source_in = SOURCE(address_in[selected_port]);
			uint8_t id_in = ID(id_svc_in[selected_port]);
			uint8_t in_table_idx = 0;
			for(int i = 0; i < CAM_SIZE; i++){
				if(table[i].used){
					uint16_t source_table = SOURCE(table[i].address);
					uint8_t id_table = ID(table[i].id_svc);

					if(id_table == id_in && source_in == source_table){
						// std::cout << "PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": IS IN TABLE" << std::endl;
						is_in_table = true;
						in_table_idx = i;
						source_idx = i;
						break;
					}
				}
			}

			Service svc = SERVICE(id_svc_in[selected_port]);
			if(!is_in_table && (svc == Service::TARGET || svc == Service::ALL)){
				/* Message not in CAM */
				bool table_full = true;
				for(int i = 0; i < CAM_SIZE; i++){
					if(!table[i].used){
						table_full = false;
						free_idx = i;
						break;
					}
				}

				if(table_full){
					std::cout << "++++++++++++++++++++++++++++++++++  CAM CHEIA  SEND LOCAL:  Address: " << router_address << std::endl;
					in_state = IN_INIT;
				} else {
					// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": will write in cam" << std::endl;
					in_state = IN_WRITE;
				}
			} else if(is_in_table && svc == Service::CLEAR && !table[in_table_idx].pending){
				// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": will erase in cam" << std::endl;
				in_state = IN_CLEAR;
			} else {
				// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": in table, ignoring" << std::endl;
				ack_out[selected_port] = true;
				in_state = IN_WAIT_REQ_DOWN;
			}
			break;
		}
		case IN_WAIT_REQ_DOWN:
		{
			if(!req_in[selected_port]){
				// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": acked down " << std::endl;
				ack_out[selected_port] = false;
				in_state = IN_INIT;
			}
			break;
		}
		case IN_WRITE:
		{
			table[free_idx].payload = payload_in[selected_port];
			table[free_idx].address = address_in[selected_port];
			table[free_idx].origin = selected_port;

			ack_out[selected_port] = true;

			// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": wrote to cam " << (int)free_idx << std::endl;

			in_state = IN_WAIT_REQ_DOWN;
			break;
		}
		case IN_CLEAR:
		{
			ack_out[selected_port] = true;
			// std::cout << "In PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": acked clear " << std::endl;
			in_state = IN_WAIT_REQ_DOWN;
			break;
		}
	}
}

void BrLiteRouter::output()
{
	if(reset){
		ack_ports.fill(false);
		for(int i = 0; i < NPORT; i++)
			req_out[i] = false;
		
		selected_line = CAM_SIZE - 1;
		out_state = OUT_INIT;
		return;
	}

	switch(out_state){
		case OUT_INIT:
		{
			if(!clear_local){
				bool has_pending = false;
				for(int i = 0; i < CAM_SIZE; i++){
					if(table[i].used && table[i].pending){
						has_pending = true;
						break;
					}
				}

				if(has_pending){
					// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": has pend " << std::endl;
					out_state = OUT_ARBITRATION;
				}
			}
			break;
		}
		case OUT_ARBITRATION:
		{
			uint8_t line = (selected_line + 1) % CAM_SIZE;
			while(line != selected_line){
				if(table[line].used && table[line].pending){
					selected_line = line;
					break;
				}
				line++;
				line %= CAM_SIZE;
			}
			// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": arbitred line " << (int)line << std::endl;
			out_state = OUT_TEST_SVC;
			break;
		}
		case OUT_TEST_SVC:
		{
			Service svc = SERVICE(table[selected_line].id_svc);
			uint16_t target = TARGET(table[selected_line].address);
			// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": target is " << (int)target << std::endl;

			if(svc == Service::CLEAR || svc == Service::ALL || (svc == Service::TARGET && target != router_address)){
				/* Propagate */
				for(int i = 0; i < NPORT - 1; i++){
					if(i != table[selected_line].origin){
						req_out[i] = true;
						id_svc_out[i] = table[selected_line].id_svc;
						address_out[i] = table[selected_line].address;
						payload_out[i] = table[selected_line].payload;
					} else {
						ack_ports[i] = true;	/* Ack on the port the packet has entered */
					}
				}

				uint16_t source = SOURCE(table[selected_line].address);
				if((svc == Service::ALL && source != router_address) || (svc == Service::TARGET && target == router_address)){
					req_out[LOCAL] = true;
					id_svc_out[LOCAL] = table[selected_line].id_svc;
					address_out[LOCAL] = table[selected_line].address;
					payload_out[LOCAL] = table[selected_line].payload;
				} else {
					ack_ports[LOCAL] = true;
				}

				// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": propagated" << std::endl;
				out_state = OUT_WAIT_ACK_PORTS;
			} else if(svc == Service::TARGET && target == router_address){
				req_out[LOCAL] = true;
				id_svc_out[LOCAL] = table[selected_line].id_svc;
				address_out[LOCAL] = table[selected_line].address;
				payload_out[LOCAL] = table[selected_line].payload;
				
				// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": sent local" << std::endl;
				out_state = OUT_SEND_LOCAL;
			} else {
				// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": invalid" << std::endl;
				out_state = OUT_INIT;
			}
			break;
		}
		case OUT_SEND_LOCAL:
		{
			if(ack_in[LOCAL]){
				uint16_t source = SOURCE(table[selected_line].address);
				uint8_t src_x = source >> 8;
				uint8_t src_y = source & 0xFF;
				uint16_t target = TARGET(table[selected_line].address);
				uint8_t tgt_x = target >> 8;
				uint8_t tgt_y = target & 0xFF;
				uint8_t svc = table[selected_line].id_svc & 0x3;
				std::cout << ">>>>>>>>>>>>>>>>> SEND LOCAL: [[" << 
					(int)src_x << " " << (int)src_y << " " <<
					(int)tgt_x << " " << (int)tgt_y << "]] " << 
					(int)svc << " " <<
					table[selected_line].payload << " " <<
					"Address: " << (int)router_address << 
					std::endl;
			}

			if(ack_in[LOCAL]){
				// std::cout << "Out PE " << (int)(router_address >> 4) << "x" << (int)(router_address & 0xF) << ": local acked" << std::endl;
				req_out[LOCAL] = false;
				out_state = OUT_WAIT_ACK_DOWN;
			}

			break;
		}
		case OUT_WAIT_ACK_PORTS:
		{
			bool ack = true;
			for(int i = 0; i < NPORT; i++){
				if(!ack_ports[i] && ack_in[i]){
					ack_ports[i] = true;
					req_out[i] = false;
				} else {
					ack &= ack_ports[i];
				}
			}

			if(ack){
				ack_ports.fill(false);

				Service svc = SERVICE(table[selected_line].id_svc);
				if(svc == Service::CLEAR){
					// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": ports acked. clearing now" << std::endl;
					out_state = OUT_CLEAR_TABLE;
				} else {
					// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": ports acked" << std::endl;
					out_state = OUT_INIT;
				}
			}
			break;
		}
		case OUT_WAIT_ACK_DOWN:
		{	
			if(!ack_in[LOCAL]){
				// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": local acked down" << std::endl;
				out_state = OUT_INIT;
			}	
			break;
		}
		case OUT_CLEAR_TABLE:
		{
			uint16_t target = TARGET(table[selected_line].address);
			if(target == router_address){
				uint16_t source = SOURCE(table[selected_line].address);
				uint8_t src_x = source >> 8;
				uint8_t src_y = source & 0xFF;
				uint8_t tgt_x = target >> 8;
				uint8_t tgt_y = target & 0xFF;
				uint8_t svc = table[selected_line].id_svc & 0x3;

				std::cout << ">>>>>>>>>>>>>>>>> end CLEAR:  [[" << 
					(int)src_x << " " << (int)src_y << " " <<
					(int)tgt_x << " " << (int)tgt_y << "]] " << 
					(int)svc << " " <<
					table[selected_line].payload <<
					std::endl;
			}

			// std::cout << "Out PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": cleared" << std::endl;

			out_state = OUT_INIT;
			break;
		}
	}
}

void BrLiteRouter::input_output()
{
	if(reset){
		for(int i = 0; i < CAM_SIZE; i++){
			table[i].pending = false;
			table[i].used = false;
		}

		current_tick = 0;

		clear_local = false;
		wrote_local = false;
		local_busy = false;
		wrote_idx = 0;
		wrote_tick = 0;

		return;
	}

	current_tick = current_tick + 1;

	if(clear_local && in_state == IN_INIT && out_state == OUT_INIT){
		clear_local = false;
		local_busy = false;
		table[wrote_idx].id_svc = (table[wrote_idx].id_svc & 0xFC) | static_cast<uint8_t>(Service::CLEAR);
		table[wrote_idx].pending = true;
		// std::cout << "PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": triggering a clear" << std::endl;
	}

	if(wrote_local && current_tick >= wrote_tick + CLEAR_INTERVAL){
		// std::cout << "PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": setting clear" << std::endl;
		clear_local = true;
		wrote_local = false;
	}

	switch(in_state){
		case IN_WRITE:
		{
			table[free_idx].used = true;
			table[free_idx].pending = true;
			table[free_idx].id_svc = id_svc_in[selected_port];
			if(selected_port == LOCAL){
				// std::cout << "PE " << (int)(router_address >> 8) << "x" << (int)(router_address & 0xFF) << ": detected local write" << std::endl;
				local_busy = true;
				wrote_local = true;
				wrote_tick = current_tick;
				wrote_idx = free_idx;
			}
			break;
		}
		case IN_CLEAR:
		{
			Service svc = SERVICE(table[source_idx].id_svc);
			if(svc != Service::CLEAR && !table[source_idx].pending){
				table[source_idx].id_svc = (table[source_idx].id_svc & 0xFC) | static_cast<uint8_t>(Service::CLEAR);
				table[source_idx].pending = true;
			}
			break;
		}
		default:
			break;
	}

	switch(out_state){
		case OUT_TEST_SVC:
		{
			table[selected_line].pending = false;
			break;
		}
		case OUT_CLEAR_TABLE:
		{
			table[selected_line].used = false;
			break;
		}
		default:
			break;
	}

}
