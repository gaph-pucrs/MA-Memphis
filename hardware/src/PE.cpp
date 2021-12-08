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
 * @brief Join all PE modules. Manipulate memory mapped registers.
 */

#include "PE.hpp"

PE::PE(sc_module_name name_, regaddress address_, std::string path_) : 
	sc_module(name_), 
	cpu("cpu", router_address),
	dmni("dmni", address_),
	br_router("brrouter", address_, path_),
	br_buffer("brbuffer"),
	br_control("brcontrol", address_),
	router_address(address_), 
	path(path_)
{
	mem_peripheral.write(0);
	end_sim_reg.write(0x00000001);

	shift_mem_page = (unsigned char) (log10(PAGE_SIZE_BYTES)/log10(2));

	cpu.clk(clock_hold);
	cpu.reset_in(reset);
	cpu.intr_in(irq);
	cpu.mem_address(cpu_mem_address);
	cpu.mem_data_w(cpu_mem_data_write);
	cpu.mem_data_r(cpu_mem_data_read);
	cpu.mem_byte_we(cpu_mem_write_byte_enable);
	cpu.mem_pause(cpu_mem_pause);
	cpu.current_page(current_page);
	
	mem = new ram("ram", (unsigned int) router_address, path);
	mem->clk(clock);
	mem->enable_a(cpu_enable_ram);
	mem->wbe_a(cpu_mem_write_byte_enable);
	mem->address_a(addr_a);
	mem->data_write_a(cpu_mem_data_write);
	mem->data_read_a(data_read_ram);
	mem->enable_b(dmni_enable_internal_ram);
	mem->wbe_b(dmni_mem_write_byte_enable);
	mem->address_b(addr_b);
	mem->data_write_b(dmni_mem_data_write);
	mem->data_read_b(mem_data_read);

	dmni.clock(clock);
	dmni.reset(reset);

	dmni.set_address(cpu_set_address);
	dmni.set_address_2(cpu_set_address_2);
	dmni.set_size(cpu_set_size);
	dmni.set_size_2(cpu_set_size_2);
	dmni.set_op(cpu_set_op);
	dmni.set_mon_qos(cpu_set_mon_qos);
	dmni.set_mon_pwr(cpu_set_mon_pwr);
	dmni.set_mon_2(cpu_set_mon_2);
	dmni.set_mon_3(cpu_set_mon_3);
	dmni.set_mon_4(cpu_set_mon_4);
	dmni.start(cpu_start);

	dmni.config_data(dmni_data_read);
	dmni.intr(ni_intr);
	dmni.send_active(dmni_send_active_sig);
	dmni.receive_active(dmni_receive_active_sig);

	dmni.mem_address(dmni_mem_address);
	dmni.mem_data_write(dmni_mem_data_write);
	dmni.mem_data_read(dmni_mem_data_read);
	dmni.mem_byte_we(dmni_mem_write_byte_enable);

	dmni.tx(tx_ni);
	dmni.data_out(data_out_ni);
	dmni.credit_i(credit_i_ni);
	dmni.rx(rx_ni);
	dmni.data_in(data_in_ni);
	dmni.credit_o(credit_o_ni);

	dmni.br_ack_mon(br_dmni_ack);
	dmni.br_req_mon(br_dmni_req);
	dmni.br_mon_svc(br_dmni_svc);
	dmni.br_producer(br_dmni_prod);
	dmni.br_address(br_dmni_addr);
	dmni.br_payload(br_payload_out_local);
	dmni.clear_task(br_dmni_clear);

	router = new router_cc("router",router_address, path);
	router->clock(clock);
	router->reset_n(reset_n);
	router->tx[EAST](tx[EAST]);
	router->tx[WEST](tx[WEST]);
	router->tx[NORTH](tx[NORTH]);
	router->tx[SOUTH](tx[SOUTH]);
	router->tx[LOCAL](rx_ni);
	router->credit_o[EAST](credit_signal[EAST]);
	router->credit_o[WEST](credit_signal[WEST]);
	router->credit_o[NORTH](credit_signal[NORTH]);
	router->credit_o[SOUTH](credit_signal[SOUTH]);

	x_address = router_address >> 8;
	y_address = router_address & 0xFF;

	router->credit_o[LOCAL](credit_i_ni);
	router->data_out[EAST](data_out[EAST]);
	router->data_out[WEST](data_out[WEST]);
	router->data_out[NORTH](data_out[NORTH]);
	router->data_out[SOUTH](data_out[SOUTH]);
	router->data_out[LOCAL](data_in_ni);
	router->rx[EAST](rx[EAST]);
	router->rx[WEST](rx[WEST]);
	router->rx[NORTH](rx[NORTH]);
	router->rx[SOUTH](rx[SOUTH]);
	router->rx[LOCAL](tx_ni);
	router->credit_i[EAST](credit_i[EAST]);
	router->credit_i[WEST](credit_i[WEST]);
	router->credit_i[NORTH](credit_i[NORTH]);
	router->credit_i[SOUTH](credit_i[SOUTH]);
	router->credit_i[LOCAL](credit_o_ni);
	router->data_in[EAST](data_in[EAST]);
	router->data_in[WEST](data_in[WEST]);
	router->data_in[NORTH](data_in[NORTH]);
	router->data_in[SOUTH](data_in[SOUTH]);
	router->data_in[LOCAL](data_out_ni);
	router->tick_counter(tick_counter);

	br_router.clock(clock);
	br_router.reset(reset);
	br_router.tick_counter(tick_counter);
	
	for(int i = 0; i < NPORT - 1; i++){
		br_router.req_in[i](br_req_in[i]);
		br_router.ack_in[i](br_ack_in[i]);
		br_router.payload_in[i](br_payload_in[i]);
		br_router.address_in[i](br_address_in[i]);
		br_router.producer_in[i](br_producer_in[i]);
		br_router.id_svc_in[i](br_id_svc_in[i]);
		br_router.ksvc_in[i](br_ksvc_in[i]);

		br_router.req_out[i](br_req_out[i]);
		br_router.ack_out[i](br_ack_out[i]);
		br_router.payload_out[i](br_payload_out[i]);
		br_router.address_out[i](br_address_out[i]);
		br_router.producer_out[i](br_producer_out[i]);
		br_router.id_svc_out[i](br_id_svc_out[i]);
		br_router.ksvc_out[i](br_ksvc_out[i]);
	}

	br_router.req_in[LOCAL](br_req_in_local);
	br_router.ack_out[LOCAL](br_ack_out_local);
	br_router.payload_in[LOCAL](br_payload_in_local);
	br_router.address_in[LOCAL](br_address_in_local);
	br_router.producer_in[LOCAL](br_producer_in_local);
	br_router.id_svc_in[LOCAL](br_id_svc_in_local);
	br_router.ksvc_in[LOCAL](br_ksvc_in_local);
	br_router.local_busy(br_local_busy);

	br_router.req_out[LOCAL](br_req_out_local);
	br_router.ack_in[LOCAL](br_ack_in_local);
	br_router.payload_out[LOCAL](br_payload_out_local);
	br_router.address_out[LOCAL](br_address_out_local);
	br_router.producer_out[LOCAL](br_producer_out_local);
	br_router.id_svc_out[LOCAL](br_id_svc_out_local);
	br_router.ksvc_out[LOCAL](br_ksvc_out_local);

	br_buffer.clock(clock);
	br_buffer.reset(reset);

	br_buffer.req_in(br_buf_req);
	br_buffer.ack_out(br_buf_ack);
	br_buffer.payload_in(br_payload_out_local);
	br_buffer.address_in(br_address_out_local);
	br_buffer.producer_in(br_producer_out_local);
	br_buffer.ksvc_in(br_ksvc_out_local);

	br_buffer.empty(br_buf_empty);
	br_buffer.read_in(br_buf_read_in);
	br_buffer.payload_out(br_buf_payload_out);
	br_buffer.producer_out(br_buf_producer_out);
	br_buffer.ksvc_out(br_buf_ksvc_out);

	br_control.clock(clock);
	br_control.reset(reset);

	br_control.payload_cfg(br_cfg_payload);
	br_control.address_cfg(br_cfg_address);
	br_control.producer_cfg(br_cfg_producer);
	br_control.id_svc_cfg(br_cfg_id_svc);
	br_control.ksvc_cfg(br_cfg_ksvc);
	br_control.start_cfg(br_cfg_start);
	br_control.data_in(br_cfg_data);

	br_control.payload_out(br_cfg_payload_out);
	br_control.address_out(br_cfg_address_out);
	br_control.producer_out(br_cfg_producer_out);
	br_control.id_svc_out(br_cfg_id_svc_out);
	br_control.ksvc_out(br_cfg_ksvc_out);
	br_control.req_out(br_cfg_req_out);
	br_control.ack_in(br_cfg_ack_in);

	SC_METHOD(reset_n_attr);
	sensitive << reset;
	
	SC_METHOD(sequential_attr);
	sensitive << clock.pos() << reset.pos();
	
	SC_METHOD(log_process);
	sensitive << clock.pos() << reset.pos();
	
	SC_METHOD(comb_assignments);
	sensitive << cpu_mem_address << dmni_mem_address << cpu_mem_address_reg << write_enable;
	sensitive << cpu_mem_data_write_reg << irq_mask_reg << irq_status;
	sensitive << time_slice << tick_counter_local;
	sensitive << dmni_send_active_sig << dmni_receive_active_sig << data_read_ram;
	sensitive << cpu_set_op << cpu_set_size << cpu_set_address << cpu_set_address_2 << cpu_set_size_2 << dmni_enable_internal_ram;
	sensitive << mem_data_read << cpu_enable_ram << cpu_mem_write_byte_enable_reg << dmni_mem_write_byte_enable;
	sensitive << dmni_mem_data_write << ni_intr << slack_update_timer;
	sensitive << br_ack_out_local << br_cfg_payload_out << br_cfg_address_out << br_cfg_id_svc_out << br_cfg_req_out;
	sensitive << br_req_out_local << br_id_svc_out_local << br_buf_ack;
	sensitive << br_address_out_local;
	sensitive << br_buf_empty;
	sensitive << br_cfg_ksvc_out;
	
	SC_METHOD(mem_mapped_registers);
	sensitive << cpu_mem_address_reg;
	sensitive << tick_counter_local;
	sensitive << data_read_ram;
	sensitive << time_slice;
	sensitive << irq_status;
	sensitive << mem_peripheral;
	sensitive << br_local_busy;
	sensitive << br_cfg_req_out;
	sensitive << br_buf_payload_out;
	sensitive << br_buf_empty;
	sensitive << br_buf_producer_out;
	sensitive << br_buf_ksvc_out;
	
	SC_METHOD(end_of_simulation);
	sensitive << end_sim_reg;

	SC_METHOD(clock_stop);
	sensitive << clock << reset.pos();

	SC_METHOD(update_credit);
	sensitive << credit_signal[EAST];
	sensitive << credit_signal[WEST];	
	sensitive << credit_signal[NORTH];	
	sensitive << credit_signal[SOUTH];
	sensitive << mem_peripheral;

}

void PE::mem_mapped_registers(){
	
	sc_uint <32 > l_cpu_mem_address_reg = cpu_mem_address_reg.read();
	
	switch(l_cpu_mem_address_reg){
		case IRQ_MASK:
			cpu_mem_data_read.write(irq_mask_reg.read());
		break;
		case IRQ_STATUS_ADDR:
			cpu_mem_data_read.write(irq_status.read());
		break;
		case TIME_SLICE_ADDR:
			cpu_mem_data_read.write(time_slice.read());
		break;
		case NET_ADDRESS:
			cpu_mem_data_read.write(router_address);
		break;
		case TICK_COUNTER_ADDR:
			cpu_mem_data_read.write(tick_counter.read());
		break;
		case DMA_SEND_ACTIVE:
			cpu_mem_data_read.write(dmni_send_active_sig.read());
		break;
		case DMA_RECEIVE_ACTIVE:
			cpu_mem_data_read.write(dmni_receive_active_sig.read());
		break;
		case MEM_REG_PERIPHERALS:
			cpu_mem_data_read.write(mem_peripheral.read());
			break;
		case BR_LOCAL_BUSY:
			cpu_mem_data_read.write(br_local_busy.read() || br_cfg_req_out.read());
			break;
		case BR_READ_PAYLOAD:
			cpu_mem_data_read.write(br_buf_payload_out.read());
			break;
		case BR_READ_PRODUCER:
			cpu_mem_data_read.write(br_buf_producer_out.read());
			break;
		case BR_READ_KSVC:
			cpu_mem_data_read.write(br_buf_ksvc_out.read());
			break;
		case BR_HAS_MESSAGE:
			cpu_mem_data_read.write(!br_buf_empty.read());
			break;
		default:
			cpu_mem_data_read.write(data_read_ram.read());
		break;
	}
}


void PE::comb_assignments(){
	sc_uint<8 > l_irq_status;
	sc_uint <32 > new_mem_address;

	new_mem_address = cpu_mem_address.read();

	/*Esse IF faz com que seja editado o valor de endereço de memoria usado pela CPU. Inserindo na parte
	 * alta do endereco o valor correto de referencia para a pagina da tarefa relativo a current_page. O valor de current_page é usado para
	 * compor a parte alta do endereço de memoria.
	 * Na prática, esse IF faz com que a mascara 0xF000..(FFF..32-shift_mem_page) seja colocada no endereco para a memoria
	 * Tudo isso porque a migracao de tarefas da pilha acaba guardando informacao de offset da pagina origem, o que corrompe o retorno de fucoes
	 */
	if (current_page.read() && ((0xFFFFFFFF << shift_mem_page) & new_mem_address)){
		new_mem_address &= (0xF0000000 | (0xFFFFFFFF >> (32-shift_mem_page)));
		new_mem_address |= current_page.read() * PAGE_SIZE_BYTES;//OFFSET
	}

	addr_a.write(new_mem_address.range(31, 2));
	addr_b.write(dmni_mem_address.read()(31,2));
	
	cpu_mem_pause.write(0);
	irq.write((((irq_status.read() & irq_mask_reg.read()) != 0x00)) ? 1  : 0 );
	cpu_set_size.write((((cpu_mem_address_reg.read() == DMA_SIZE) && (write_enable.read() == 1))) ? 1  : 0 );
	cpu_set_address.write((((cpu_mem_address_reg.read() == DMA_ADDR) && (write_enable.read() == 1))) ? 1  : 0 );
	cpu_set_size_2.write((((cpu_mem_address_reg.read() == DMA_SIZE_2) && (write_enable.read() == 1))) ? 1  : 0 );
	cpu_set_address_2.write((((cpu_mem_address_reg.read() == DMA_ADDR_2) && (write_enable.read() == 1))) ? 1  : 0 );
	cpu_set_op.write((((cpu_mem_address_reg.read() == DMA_OP) && (write_enable.read() == 1))) ? 1  : 0 );
	cpu_start.write((((cpu_mem_address_reg.read() == START_DMA) && (write_enable.read() == 1))) ? 1  : 0 );
	dmni_data_read.write( cpu_mem_data_write_reg.read());
	dmni_mem_data_read.write(mem_data_read.read());
	write_enable.write(((cpu_mem_write_byte_enable_reg.read() != 0)) ? 1  : 0 );
	cpu_enable_ram.write(((cpu_mem_address.read()(30,28 ) == 0)) ? 1  : 0 );
	dmni_enable_internal_ram.write(1);
	end_sim_reg.write((((cpu_mem_address_reg.read() == END_SIM) && (write_enable.read() == 1))) ? 0x00000000 : 0x00000001);	
	
	br_cfg_payload = cpu_mem_address_reg.read() == BR_PAYLOAD && write_enable;
	br_cfg_address = cpu_mem_address_reg.read() == BR_TARGET && write_enable;
	br_cfg_producer = cpu_mem_address_reg.read() == BR_PRODUCER && write_enable;
	br_cfg_id_svc = cpu_mem_address_reg.read() == BR_SERVICE && write_enable;
	br_cfg_start = cpu_mem_address_reg.read() == BR_START && write_enable;
	br_cfg_data = cpu_mem_data_write_reg.read();
	br_buf_read_in = cpu_mem_address_reg.read() == BR_READ_PAYLOAD;
	br_cfg_ksvc = cpu_mem_address_reg.read() == BR_KSVC && write_enable;

	cpu_set_mon_qos = cpu_mem_address_reg.read() == MON_PTR_QOS && write_enable;
	cpu_set_mon_pwr = cpu_mem_address_reg.read() == MON_PTR_PWR && write_enable;
	cpu_set_mon_2 = cpu_mem_address_reg.read() == MON_PTR_2 && write_enable;
	cpu_set_mon_3 = cpu_mem_address_reg.read() == MON_PTR_3 && write_enable;
	cpu_set_mon_4 = cpu_mem_address_reg.read() == MON_PTR_4 && write_enable;

	br_buf_req = br_req_out_local && BrLiteRouter::SERVICE(br_id_svc_out_local) >= BrLiteRouter::Service::TARGET;
	br_ack_in_local = br_buf_ack || br_dmni_ack;

	br_payload_in_local = br_cfg_payload_out;
	br_address_in_local = br_cfg_address_out;
	br_producer_in_local = br_cfg_producer_out;
	br_id_svc_in_local = br_cfg_id_svc_out;
	br_ksvc_in_local = br_cfg_ksvc_out;
	br_req_in_local = br_cfg_req_out;
	br_cfg_ack_in = br_ack_out_local;

	br_dmni_req = br_req_out_local && BrLiteRouter::SERVICE(br_id_svc_out_local) < BrLiteRouter::Service::TARGET;
	br_dmni_svc = br_id_svc_out_local & 0x7;
	br_dmni_addr = br_address_out_local;
	br_dmni_prod = br_producer_out_local;
	br_dmni_clear = cpu_mem_address_reg.read() == DMNI_CLEAR_MONITOR && write_enable;

	l_irq_status[7] = 0; //unused
	l_irq_status[6] = !br_buf_empty;
	l_irq_status[5] = ni_intr.read();
	l_irq_status[4] = 0; //unused
	l_irq_status[3] = (time_slice.read() == 1) ? 1  : 0;
	l_irq_status[2] = 0; //unused
	l_irq_status[1] = (!dmni_send_active_sig.read() && slack_update_timer.read() == SLACK_MONITOR_WINDOW) ? 1  : 0;
	l_irq_status[0] = (!dmni_send_active_sig.read() && pending_service.read());
	
	irq_status.write(l_irq_status);
}

void PE::reset_n_attr(){
	reset_n.write(!reset.read());
}

void PE::sequential_attr(){

	FILE *fp;

	if (reset.read() == 1) {
		cpu_mem_address_reg.write(0);
		cpu_mem_data_write_reg.write(0);
		cpu_mem_write_byte_enable_reg.write(0);
		irq_mask_reg.write(0);
		time_slice.write(0);
		tick_counter.write(0);
		pending_service.write(0);
		slack_update_timer.write(0);
		mem_peripheral.write(0);
	} else {

		if(cpu_mem_pause.read() == 0) {
			cpu_mem_address_reg.write(cpu_mem_address.read());
			
			cpu_mem_data_write_reg.write(cpu_mem_data_write.read());
			
			cpu_mem_write_byte_enable_reg.write(cpu_mem_write_byte_enable.read());
			
			if(cpu_mem_address_reg.read()==IRQ_MASK && write_enable.read()==1){
				irq_mask_reg.write(cpu_mem_data_write_reg.read()(7,0));
			}

			if (time_slice.read() > 1) {
				time_slice.write(time_slice.read() - 1);
			}

			//************** pending service implementation *******************
			if (cpu_mem_address_reg.read() == PENDING_SERVICE_INTR && write_enable.read() == 1){
				if (cpu_mem_data_write_reg.read() == 0){
					pending_service.write(0);
				} else {
					pending_service.write(1);
				}
			}
			//*********************************************************************

		}

		//****************** slack time monitoring **********************************
		if (cpu_mem_address_reg.read() == SLACK_TIME_MONITOR && write_enable.read() == 1){
			slack_update_timer.write(cpu_mem_data_write_reg.read());
		} else if (slack_update_timer.read() < SLACK_MONITOR_WINDOW){
			slack_update_timer.write(slack_update_timer.read() + 1);
		}
		//*********************************************************************

		//************** simluation-time debug implementation *******************
		if (cpu_mem_address_reg.read() == DEBUG && write_enable.read() == 1){
			sprintf(aux, "%s/log/log%dx%d.txt", path.c_str(), (unsigned int) router_address.range(15,8), (unsigned int) router_address.range(7,0));
			fp = fopen (aux, "a");

			bool end = false;
			uint32_t address = cpu_mem_data_write_reg.read()/4;
			fprintf(fp, "%s", &(mem->ram_data[address]));

			fclose (fp);
		}
		if (cpu_mem_address_reg.read() == 0x20000001U && write_enable.read() == 1){
			sprintf(aux, "%s/log/log%dx%d.txt", path.c_str(), (unsigned int) router_address.range(15,8), (unsigned int) router_address.range(7,0));
			fp = fopen (aux, "a");
			fprintf(fp, "%c", (char)cpu_mem_data_write_reg.read());
			fclose (fp);
		}

		//************ NEW DEBBUG AND REPORT logs - they are used by Memphis Debbuger Tool********
		if (write_enable.read()==1){

			if(cpu_mem_address_reg.read() == MEM_REG_PERIPHERALS){
				mem_peripheral.write(cpu_mem_data_write_reg.read());
				// std::cout << "PE " << (int)x_address << "x" << (int)y_address <<": MEM_REG_PERIPHERALS = " << cpu_mem_data_write_reg.read() << std::endl;
			}
				
			/* TASK_TERMINATED report implementation */
			if(cpu_mem_address_reg.read() == TASK_TERMINATED){
				sprintf(aux, "%s/debug/traffic_router.txt", path.c_str());
				fp = fopen(aux, "a");
				fprintf(fp, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\n", (unsigned int)tick_counter.read(), (unsigned int)router_address, 0x70, 0, 0, 4*2, -1, (unsigned int)cpu_mem_data_write_reg.read());
				fclose(fp);
			}

			//************** Scheduling report implementation *******************
			if (cpu_mem_address_reg.read() == SCHEDULING_REPORT) {
				sprintf(aux, "%s/debug/scheduling_report.txt", path.c_str());
				fp = fopen (aux, "a");
				sprintf(aux, "%d\t%d\t%d\n", (unsigned int)router_address, (unsigned int)cpu_mem_data_write_reg.read(), (unsigned int)tick_counter.read());
				fprintf(fp,"%s",aux);
				fclose (fp);
			}
			//**********************************************************************

			//************** PIPE and request debug implementation *******************
			if (cpu_mem_address_reg.read() == ADD_PIPE_DEBUG ) {
				sprintf(aux, "%s/debug/pipe/%d.txt", path.c_str(), (unsigned int)router_address);
				fp = fopen (aux, "a");
				sprintf(aux, "add\t%d\t%d\t%d\n", (unsigned int)(cpu_mem_data_write_reg.read() >> 16), (unsigned int)(cpu_mem_data_write_reg.read() & 0xFFFF), (unsigned int)tick_counter.read());
				fprintf(fp,"%s",aux);
				fclose (fp);

			} else if (cpu_mem_address_reg.read() == REM_PIPE_DEBUG ) {
				sprintf(aux, "%s/debug/pipe/%d.txt", path.c_str(), (unsigned int)router_address);
				fp = fopen (aux, "a");
				sprintf(aux, "rem\t%d\t%d\t%d\n", (unsigned int)(cpu_mem_data_write_reg.read() >> 16), (unsigned int)(cpu_mem_data_write_reg.read() & 0xFFFF), (unsigned int)tick_counter.read());
				fprintf(fp,"%s",aux);
				fclose (fp);
			} else if (cpu_mem_address_reg.read() == ADD_REQUEST_DEBUG ) {
				sprintf(aux, "%s/debug/request/%d.txt", path.c_str(), (unsigned int)router_address);
				fp = fopen (aux, "a");
				sprintf(aux, "add\t%d\t%d\t%d\n", (unsigned int)(cpu_mem_data_write_reg.read() >> 16), (unsigned int)(cpu_mem_data_write_reg.read() & 0xFFFF), (unsigned int)tick_counter.read());
				fprintf(fp,"%s",aux);
				fclose (fp);

			} else if (cpu_mem_address_reg.read() == REM_REQUEST_DEBUG ) {
				sprintf(aux, "%s/debug/request/%d.txt", path.c_str(), (unsigned int)router_address);
				fp = fopen (aux, "a");
				sprintf(aux, "rem\t%d\t%d\t%d\n", (unsigned int)(cpu_mem_data_write_reg.read() >> 16), (unsigned int)(cpu_mem_data_write_reg.read() & 0xFFFF), (unsigned int)tick_counter.read());
				fprintf(fp,"%s",aux);
				fclose (fp);
			}
		}
		//**********************************************************************


		if ((cpu_mem_address_reg.read() == TIME_SLICE_ADDR) and (write_enable.read()==1) ) {
			time_slice.write(cpu_mem_data_write_reg.read());
  		}
  		
  		tick_counter.write((tick_counter.read() + 1) );

	}
}

void PE::end_of_simulation(){
	if (end_sim_reg.read() == 0x00000000){
		cout << "END OF ALL APPLICATIONS!!!" << endl;
		cout << "Simulation time: " << (float) ((tick_counter.read() * 10.0f) / 1000.0f / 1000.0f) << "ms" << endl;
		sc_stop();
	}
}

void PE::log_process(){
	if (reset.read() == 1) {
		log_interaction=1;
		instant_instructions = 0;		
		aux_instant_instructions = 0;

		logical_instant_instructions = 0;
		jump_instant_instructions = 0;
		branch_instant_instructions = 0;
		move_instant_instructions = 0;
		other_instant_instructions = 0;
		arith_instant_instructions = 0;
		load_instant_instructions = 0;
		shift_instant_instructions = 0;
		nop_instant_instructions = 0;
		mult_div_instant_instructions = 0;

	} else {

		if(tick_counter.read() == 100000*log_interaction) {
			
			char aux_file[255];
			sprintf(aux_file, "%s/log_tasks.txt", path.c_str());
			fp = fopen (aux_file, "a+");
			
			aux_instant_instructions = cpu.global_inst - instant_instructions;

			sprintf(aux, "%d,%lu,%lu,%lu\n",  (int)router_address,cpu.global_inst,aux_instant_instructions,100000*log_interaction);

			instant_instructions = cpu.global_inst;

		
			fprintf(fp,"%s",aux);
		
			fclose(fp); 
			fp = NULL;

			sprintf(aux_file, "%s/log_tasks_full.txt", path.c_str());
			fp = fopen (aux_file, "a+");


			fprintf(fp,"%d ",(int)router_address);

			aux_instant_instructions = cpu.logical_inst - logical_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);
			
			logical_instant_instructions = cpu.logical_inst;

			aux_instant_instructions = cpu.jump_inst - jump_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			jump_instant_instructions = cpu.jump_inst;

			aux_instant_instructions = cpu.branch_inst - branch_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			branch_instant_instructions = cpu.branch_inst;

			aux_instant_instructions = cpu.move_inst - move_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			move_instant_instructions = cpu.move_inst;

			aux_instant_instructions = cpu.other_inst - other_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			other_instant_instructions = cpu.other_inst;

			aux_instant_instructions = cpu.arith_inst - arith_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			arith_instant_instructions = cpu.arith_inst;

			aux_instant_instructions = cpu.load_inst - load_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			load_instant_instructions = cpu.load_inst;

			aux_instant_instructions = cpu.shift_inst - shift_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			shift_instant_instructions = cpu.shift_inst;

			aux_instant_instructions = cpu.nop_inst - nop_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);

			nop_instant_instructions = cpu.nop_inst;

			aux_instant_instructions = cpu.mult_div_inst - mult_div_instant_instructions;

			fprintf(fp,"%lu ",aux_instant_instructions);
			
			mult_div_instant_instructions = cpu.mult_div_inst;

			fprintf(fp,"%lu",100000*log_interaction);
			fprintf(fp,"\n");
		
		
			fclose(fp); 
			fp = NULL;


			log_interaction++;
		}
	}
}


void PE::clock_stop(){

	if (reset.read() == 1) {
		tick_counter_local.write(0);
		clock_aux = true;
	}

	if((cpu_mem_address_reg.read() == CLOCK_HOLD) && (write_enable.read() == 1)){
		clock_aux = false;

	//} else if((rx_ni.read() == 1 || ni_intr.read() == 1) || time_slice.read() == 1 || irq_status.read().range(1,1)){
	} else if(!br_buf_empty || ni_intr.read() == 1 || time_slice.read() == 1 || irq_status.read().range(1,1)){
		clock_aux = true;
	}

	if((clock and clock_aux) == true){
		tick_counter_local.write((tick_counter_local.read() + 1) );
	}

	clock_hold.write(clock and clock_aux);

}

void PE::update_credit()
{
	int seq_addr = (int)x_address * N_PE_X + (int)y_address;

	/* First column, has WEST port connected to peripherals */
	if(x_address == 0){
		/* If has MA_Injector or released peripherals, update signal */
		if(io_port[seq_addr] == WEST && (seq_addr == MAINJECTOR || mem_peripheral.read() == 1))
			credit_o[WEST].write(credit_signal[WEST]);
		else 
			credit_o[WEST].write(0);
	} else {
		/* If not first column, connect WEST port to many-core */
		credit_o[WEST].write(credit_signal[WEST]);
	}
	if(x_address == (N_PE_X - 1)){
		if(io_port[seq_addr] == EAST && (seq_addr == MAINJECTOR || mem_peripheral.read() == 1))
			credit_o[EAST].write(credit_signal[EAST]);
		else
			credit_o[EAST].write(0);
	} else {
		credit_o[EAST].write(credit_signal[EAST]);
	}
	if(y_address == 0){
		if(io_port[seq_addr] == SOUTH && (seq_addr == MAINJECTOR || mem_peripheral.read() == 1))
			credit_o[SOUTH].write(credit_signal[SOUTH]);
		else 
			credit_o[SOUTH].write(0);
	} else {
		credit_o[SOUTH].write(credit_signal[SOUTH]);
	}
	if(y_address == (N_PE_Y - 1)){
		if(io_port[seq_addr] == NORTH && (seq_addr == MAINJECTOR || mem_peripheral.read() == 1))
			credit_o[NORTH].write(credit_signal[NORTH]);
		else
			credit_o[NORTH].write(0);
	} else {
		credit_o[NORTH].write(credit_signal[NORTH]);
	}
  
}
