/**
 * MA-Memphis
 * @file BrLiteRouter.hpp
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2021
 * 
 * @brief Light BrNoC router module -- Removed backtrack (unicast)
 */

#pragma once

#include <array>
#include <systemc.h>

#include "standards.h"

SC_MODULE(BrLiteRouter){
public:
	enum class Service : uint8_t {
		MON_QOS,
		MON_PWR,
		MON_2,
		MON_3,
		MON_4,
		TARGET,
		ALL,
		CLEAR
	};

	static constexpr uint16_t SOURCE(uint32_t address)
	{
		return address >> 16;
	}

	static constexpr uint16_t TARGET(uint32_t address)
	{
		return address;
	}

	static constexpr uint8_t ID(uint8_t id_svc)
	{
		return id_svc >> 3;
	}

	static constexpr Service SERVICE(uint8_t id_svc)
	{
		return static_cast<Service>(id_svc & 0x7);
	}

	/* Router signals */
	sc_in<bool>	clock;
	sc_in<bool>	reset;

	/* Data Inputs */
	sc_in<uint32_t>		payload_in[NPORT];
	sc_in<uint32_t>		address_in[NPORT];
	sc_in<uint8_t>		id_svc_in[NPORT];
	sc_in<bool>			req_in[NPORT];
	sc_out<bool>		ack_out[NPORT];

	/* Data Outputs */
	sc_out<uint32_t>	payload_out[NPORT];
	sc_out<uint32_t>	address_out[NPORT];
	sc_out<uint8_t>		id_svc_out[NPORT];
	sc_out<bool>		req_out[NPORT];
	sc_in<bool>			ack_in[NPORT];

	sc_out<bool> local_busy;

	SC_HAS_PROCESS(BrLiteRouter);
	BrLiteRouter(sc_module_name _name, uint16_t _address);

private:
	static const uint8_t CAM_SIZE = 8;
	static const uint16_t CLEAR_INTERVAL = 150;

	typedef struct _table {
		sc_signal<bool>		used;
		sc_signal<bool>		pending;
		sc_signal<uint8_t>	origin;
		sc_signal<uint8_t>	id_svc;
		sc_signal<uint32_t>	address;
		sc_signal<uint32_t>	payload;
	} table_t;

	enum IN_FSM {
		IN_INIT,
		IN_ARBITRATION,
		IN_TEST_SPACE,
		IN_WRITE,
		IN_CLEAR,
		IN_WAIT_REQ_DOWN,
	};

	enum OUT_FSM {
		OUT_INIT,
		OUT_ARBITRATION,
		OUT_TEST_SVC,
		OUT_WAIT_ACK_PORTS,
		OUT_CLEAR_TABLE,
		OUT_SEND_LOCAL,
		OUT_WAIT_ACK_DOWN,
	};

	sc_signal<uint8_t>	selected_line;
	sc_signal<uint8_t>	selected_port;
	sc_signal<uint8_t>	source_idx;
	sc_signal<uint8_t>	free_idx;
	sc_signal<bool>		clear_local;

	sc_signal<enum IN_FSM>	in_state;
	sc_signal<enum OUT_FSM> out_state;

	table_t table[CAM_SIZE];

	uint16_t router_address;

	std::array<bool,	NPORT>	ack_ports;
	bool wrote_local;
	uint8_t wrote_idx;
	uint32_t wrote_tick;
	uint32_t current_tick;

	void input();
	void output();
	void input_output();
};
