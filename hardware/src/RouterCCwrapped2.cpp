//------------------------------------------------------------------------------------------------
//
//  DISTRIBUTED HEMPS -  5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2015
//
//  Source name:  RouterCCwrapped2.cpp
//
//  Brief description: wrapper for the RouterCC module described in VHDL
//
//------------------------------------------------------------------------------------------------

#include "RouterCCwrapped2.h"

//input signals
//read from in, write in internal

void RouterCCwrapped::upd_fail_in(){
	int i;
	sc_uint<NPORT > l_fail_in_internal;

	for(i=0;i<NPORT;i++){
		l_fail_in_internal[i] = fail_in[i].read();
	}

	fail_in_internal.write(l_fail_in_internal);
}

void RouterCCwrapped::upd_clock_rx(){
	int i;
	sc_uint<NPORT > l_clock_rx_internal;

	 for(i=0;i<NPORT;i++){
		 l_clock_rx_internal[i] = clock_rx[i].read();
	 }

	 clock_rx_internal.write(l_clock_rx_internal);
}

void RouterCCwrapped::upd_eop_in(){
	int i;
	sc_uint<NPORT > l_eop_in_internal;

		for(i=0;i<NPORT;i++){
				//modified to add faults
				l_eop_in_internal[i] = eop_in[i].read();// & ~fail_in[i].read();
	 }
	 eop_in_internal.write(l_eop_in_internal);
}

void RouterCCwrapped::upd_rx(){
	int i;
	sc_uint<NPORT > l_rx_internal;

		for(i=0;i<NPORT;i++){
				//modified to add faults
				l_rx_internal[i] = rx[i].read() & ~fail_out[i].read();
	 }
	 rx_internal.write(l_rx_internal);
}

void RouterCCwrapped::upd_credit_i(){
	int i;
	sc_uint<NPORT > l_credit_i_internal;

	 for(i=0;i<NPORT;i++){
		//modified to add faults
		l_credit_i_internal[i] = (credit_i[i].read() | fail_in[i].read());
	 }

	 credit_i_internal.write(l_credit_i_internal);
}


//output signals
//read from internal, write in output

void RouterCCwrapped::upd_fail_out(){
	int i;
	sc_uint<NPORT > l_fail_out_internal;

	l_fail_out_internal = fail_out_internal.read();

	for(i=0;i<NPORT;i++){
		fail_out[i].write(l_fail_out_internal[i]);
	}
}

void RouterCCwrapped::upd_credit_o(){
	int i;
	sc_uint<NPORT > l_credit_o_internal;

	l_credit_o_internal = credit_o_internal.read();

	for(i=0;i<NPORT;i++){
			//modified to add faults
			credit_o[i].write(l_credit_o_internal[i] | fail_out[i].read());
	}
}

void RouterCCwrapped::upd_clock_tx(){
	int i;
	sc_uint<NPORT > l_clock_tx_internal;

	l_clock_tx_internal = clock_tx_internal.read();

	for(i=0;i<NPORT;i++){
		clock_tx[i].write(l_clock_tx_internal[i]);
	}
}

void RouterCCwrapped::upd_tx(){
	int i;
	sc_uint<NPORT > l_tx_internal;

	l_tx_internal = tx_internal.read();

	for(i=0;i<NPORT;i++){
		//modified to add faults
		tx[i].write(l_tx_internal[i] & ~fail_in[i].read());
	}
}

void RouterCCwrapped::upd_eop_out(){
	int i;
	sc_uint<NPORT > l_eop_out_internal;

	l_eop_out_internal = eop_out_internal.read();

	for(i=0;i<NPORT;i++){
		//modified to add faults
		eop_out[i].write(l_eop_out_internal[i]);// & ~fail_out[i].read());
	}
}


void RouterCCwrapped::traffic_monitor(){
	int i,aux2;
	char aux[255]; 
	unsigned char SM_traffic_monitor_old[NPORT];
	FILE *fp;

	if(reset.read() == 1){
		for(i = 0; i < NPORT; i++) {
			bandwidth_allocation[i] = 0;
			SM_traffic_monitor[i] = 11;
			counter_target[i] = 0;
			SR_found[i] = 0;
		}
	}
	else if (clock.read() == 1){

		for(i = 0; i < NPORT; i++){
		
			//New flit coming
			if(rx[i].read() && credit_o[i]){

				if (payload_counter[i] != 0)
					payload_counter[i]--;

				//how many clock ticks were spent 
				//from header untill the last packet piece
				bandwidth_allocation[i]++;

				if( (( address == 260 || address == 516) && SM_traffic_monitor[i] != SM_traffic_monitor_old[i]) && i==9){
					aux2 = SM_traffic_monitor[i]; 
					//		cout<<"SM_traffic_monitor[" << i << "]: " << aux2 << endl;
					//		cout<<"    target:" << target_router[i] << endl;
					//		cout<<"   service: " << service[i] << endl;
					//		cout<<"    length: " << payload[i] << endl;
				}
				

				switch(SM_traffic_monitor[i])
				{

					case 11: //consumer_id low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						target_router[i] = data_in[i].read();
						header_time[i] = (unsigned int)tick_counter.read();
						if ((target_router[i] >> 12) == 7){
							SR_found[i] = 1;
						}
						else{//XY
							if (SR_found[i]){
								SM_traffic_monitor[i] = 12;
								payload[i] = data_in[i].read();
							}
							else{
								SM_traffic_monitor[i] = 12;
							}
						}	
					break;

					case 12: //consumer_id low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						target_router[i] = ((target_router[i] << 16) + data_in[i].read());
						if (SR_found[i])
								SM_traffic_monitor[i] = 2;
						else
								SM_traffic_monitor[i] = 0;
					break;


					case 0: //Header high
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
					//	target_router[i] = data_in[i].read();
						header_time[i] = (unsigned int)tick_counter.read();
						if ((target_router[i] >> 12) == 7){
							SR_found[i] = 1;
						}
						else{//XY
							if (SR_found[i]){
								SM_traffic_monitor[i] = 3;
								payload[i] = data_in[i].read();
							}
							else{
								SM_traffic_monitor[i] = 1;
							}
						}		
						break;					

					case 1: //Header low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						SM_traffic_monitor[i] = 2;
					//	target_router[i] = ((target_router[i] << 16) + data_in[i].read());
						break;

					case 2: //Payload Size
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						payload[i] = data_in[i].read();
						if ((payload[i] >> 12) == 7)
								SM_traffic_monitor[i] = 2;
						else{					
								SM_traffic_monitor[i] = 3;
						}
						break;          
					case 3: //Payload Size low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						payload[i] = ((payload[i] << 16) + data_in[i].read())*2;
						SM_traffic_monitor[i] = 4;
						payload_counter[i] = payload[i]-2;
						if (SR_found[i]){
							payload_counter[i] -=2 ;						
						}
						break;          

					case 4: //Service high
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						service[i] = data_in[i].read();
						SM_traffic_monitor[i] = 5;
						break;
						
					case 5: //Service low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						service[i] = (service[i] << 16) + data_in[i].read();

						/*switch(data_in[i].read()){
						case 0x10:
						case 0x20:
						case 0x40:
						case 0x50:
						case 0x60:
						case 0x70:
						case 0x80:
						case 0x90:
						case 0x130:
						case 0x140:
						case 0x150:
						case 0x160:
						case 0x170:
						case 0x180:
						case 0x190:
						case 0x200:
						break;
						default:
							cout << (unsigned int)address << " ############ ERROR - SERVICE UNKNOWED: " << data_in[i].read() << "Inport " << i << " time: " << (unsigned int)tick_counter.read() << endl;
						}*/

						if(service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x221 && service[i] != 0x10 && service[i] != 0x20 )
							SM_traffic_monitor[i] = 10;
						else
							SM_traffic_monitor[i] = 6;
						break;	

					case 6: //If is task_allocation high
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						task_id[i] = data_in[i].read();
						SM_traffic_monitor[i] = 7;

						break;
						
					case 7: //If is task_allocation low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						task_id[i] = (task_id[i] << 16) + data_in[i].read();
						if (service[i] == 0x10 || service[i] == 0x20)
							SM_traffic_monitor[i] = 8;
						else
							SM_traffic_monitor[i] = 10;

						break;	
						

					case 8: //consumer_id high
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						consumer_id[i] = data_in[i].read();
						SM_traffic_monitor[i] = 9;
						break;
						
					case 9: //consumer_id low
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						consumer_id[i] = (consumer_id[i] << 16) + data_in[i].read();
						SM_traffic_monitor[i] = 10;
						break;	

					case 10:
						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
						counter_target[i]++;
						if (counter_target[i] == 2 && SR_found[i] == 1) {
							target_router[i] = data_in[i].read();
						}

						if (payload_counter[i] == 0 ){
								
							//Store in aux the C's string way
							sprintf(aux, "%s/debug/traffic_router.txt", path.c_str());

							//unsigned int aux2 = (unsigned int)address;
							//unsigned int newAdress = ((aux2 >> 8) << 4) | (aux2 & 0xFF);
							unsigned int newAdress = (unsigned int)address;
							//aux2 = target_router[i];
							//unsigned int targetRouter = ((aux2 >> 8) << 4) | (aux2 & 0xFF);
							unsigned int targetRouter = target_router[i];
																								
							// Open a file called "aux" deferred on append mode
							fp = fopen (aux, "a");

								// if (service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x221 && service[i] != 0x10 && service[i] != 0x20 ){
								// 	sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter);
								// 	// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter);
								// } else {
								// 	if (service[i] == 0x10 || service[i] == 0x20 )
								// 		sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i], consumer_id[i]);
								// 		// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i], consumer_id[i]);
								// 	else
								// 		sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i]);
								// 		// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i]);

								// }
								if (service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x23 && service[i] != 0x00 && service[i] != 0x01 && service[i] != 0x31){
									sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i*2, targetRouter);
								} else {
									if (service[i] == 0x00 || service[i] == 0x01 || service[i] == 0x31)
										sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i*2, targetRouter, task_id[i], consumer_id[i]);
									else
										sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i*2, targetRouter, task_id[i]);

								}

								fprintf(fp,"%s",aux);
								fclose (fp);
								target_router[i] = 0;
								counter_target[i] = 0;
								SR_found[i] = 0;
								payload[i] = 0;
								payload_counter[i] = 0; 
								bandwidth_allocation[i] = 0;
								SM_traffic_monitor[i] = 11;
						}
						break;
				}
			}else if(bandwidth_allocation[i] > 0){
				bandwidth_allocation[i]++;
			}
		}

	}
}





//
// WORK XY - fail SR
//void RouterCCwrapped::traffic_monitor(){
//	int i,aux2;
//	char aux[255]; 
//	unsigned char SM_traffic_monitor_old[NPORT];
//	FILE *fp;
//
//	if(reset.read() == 1){
//		for(i = 0; i < NPORT; i++) {
//			bandwidth_allocation[i] = 0;
//			SM_traffic_monitor[i] = 11;
//			counter_target[i] = 0;
//			SR_found[i] = 0;
//		}
//	}
//	else if (clock.read() == 1){
//
//		for(i = 0; i < NPORT; i++){
//		
//			//New flit coming
//			if(rx[i].read() && credit_o[i]){
//
//				if (payload_counter[i] != 0)
//					payload_counter[i]--;
//
//				//how many clock ticks were spent 
//				//from header untill the last packet piece
//				bandwidth_allocation[i]++;
//
//		if( (( address == 3 || address == 4) && SM_traffic_monitor[i] != SM_traffic_monitor_old[i]) && i==9){
//				aux2 = SM_traffic_monitor[i]; 
//				cout<<"SM_traffic_monitor[" << i << "]: " << aux2 << endl;
//				cout<<"    target:" << target_router[i] << endl;
//				cout<<"   service: " << service[i] << endl;
//				cout<<"    length: " << payload[i] << endl;
//			}
//				
//
//				switch(SM_traffic_monitor[i])
//				{
//
//					case 11: //consumer_id low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						target_router[i] = data_in[i].read();
//						header_time[i] = (unsigned int)tick_counter.read();
//						if ((target_router[i] >> 12) == 7){
//							SR_found[i] = 1;
//						}
//						else{//XY
//							if (SR_found[i]){
//								SM_traffic_monitor[i] = 12;
//								payload[i] = data_in[i].read();
//							}
//							else{
//								SM_traffic_monitor[i] = 12;
//							}
//						}	
//					break;
//
//					case 12: //consumer_id low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						SM_traffic_monitor[i] = 0;
//						target_router[i] = ((target_router[i] << 16) + data_in[i].read());
//					break;
//
//
//					case 0: //Header high
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//					//	target_router[i] = data_in[i].read();
//						header_time[i] = (unsigned int)tick_counter.read();
//						if ((target_router[i] >> 12) == 7){
//							SR_found[i] = 1;
//						}
//						else{//XY
//							if (SR_found[i]){
//								SM_traffic_monitor[i] = 3;
//								payload[i] = data_in[i].read();
//							}
//							else{
//								SM_traffic_monitor[i] = 1;
//							}
//						}		
//						break;					
//
//					case 1: //Header low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						SM_traffic_monitor[i] = 2;
//					//	target_router[i] = ((target_router[i] << 16) + data_in[i].read());
//						break;
//
//					case 2: //Payload Size
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						payload[i] = data_in[i].read();
//						SM_traffic_monitor[i] = 3;
//						break;          
//					case 3: //Payload Size low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						payload[i] = ((payload[i] << 16) + data_in[i].read())*2;
//						SM_traffic_monitor[i] = 4;
//						payload_counter[i] = payload[i]-2;
//						if (SR_found[i]){
//							payload_counter[i] -=2 ;						
//						}
//						break;          
//
//					case 4: //Service high
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						service[i] = data_in[i].read();
//						SM_traffic_monitor[i] = 5;
//						break;
//						
//					case 5: //Service low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						service[i] = (service[i] << 16) + data_in[i].read();
//
//						/*switch(data_in[i].read()){
//						case 0x10:
//						case 0x20:
//						case 0x40:
//						case 0x50:
//						case 0x60:
//						case 0x70:
//						case 0x80:
//						case 0x90:
//						case 0x130:
//						case 0x140:
//						case 0x150:
//						case 0x160:
//						case 0x170:
//						case 0x180:
//						case 0x190:
//						case 0x200:
//						break;
//						default:
//							cout << (unsigned int)address << " ############ ERROR - SERVICE UNKNOWED: " << data_in[i].read() << "Inport " << i << " time: " << (unsigned int)tick_counter.read() << endl;
//						}*/
//
//						if(service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x221 && service[i] != 0x10 && service[i] != 0x20)
//							SM_traffic_monitor[i] = 10;
//						else
//							SM_traffic_monitor[i] = 6;
//						break;	
//
//					case 6: //If is task_allocation high
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						task_id[i] = data_in[i].read();
//						SM_traffic_monitor[i] = 7;
//
//						break;
//						
//					case 7: //If is task_allocation low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						task_id[i] = (task_id[i] << 16) + data_in[i].read();
//						if (service[i] == 0x10 || service[i] == 0x20)
//							SM_traffic_monitor[i] = 8;
//						else
//							SM_traffic_monitor[i] = 10;
//
//						break;	
//						
//
//					case 8: //consumer_id high
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						consumer_id[i] = data_in[i].read();
//						SM_traffic_monitor[i] = 9;
//						break;
//						
//					case 9: //consumer_id low
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						consumer_id[i] = (consumer_id[i] << 16) + data_in[i].read();
//						SM_traffic_monitor[i] = 10;
//						break;	
//
//					case 10:
//						SM_traffic_monitor_old[i] = SM_traffic_monitor[i];
//						counter_target[i]++;
//						if (counter_target[i] == 2 && SR_found[i] == 1) {
//							target_router[i] = data_in[i].read();
//						}
//
//						if (payload_counter[i] == 0 ){
//							
//						//Store in aux the C's string way
//						sprintf(aux, "debug/traffic_router.txt");
//
//						//unsigned int aux2 = (unsigned int)address;
//						//unsigned int newAdress = ((aux2 >> 8) << 4) | (aux2 & 0xFF);
//						unsigned int newAdress = (unsigned int)address;
//						//aux2 = target_router[i];
//						//unsigned int targetRouter = ((aux2 >> 8) << 4) | (aux2 & 0xFF);
//						unsigned int targetRouter = target_router[i];
//																							
//						// Open a file called "aux" deferred on append mode
//						fp = fopen (aux, "a");
//
//							if (service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x221 && service[i] != 0x10 && service[i] != 0x20){
//								sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter);
//								// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter);
//							} else {
//								if (service[i] == 0x10 || service[i] == 0x20)
//									sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i], consumer_id[i]);
//									// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i], consumer_id[i]);
//								else
//									sprintf(aux, "%d\t%d\t%x\t%d\t%d\t%d\t%d\t%d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i]);
//									// sprintf(aux, "%8d\t%5d\t%3x\t%5d\t%5d\t%5d\t%5d\t%5d\n", header_time[i], newAdress, service[i], payload[i], bandwidth_allocation[i], i, targetRouter, task_id[i]);
//
//							}
//
//						fprintf(fp,"%s",aux);
//						fclose (fp);
//						target_router[i] = 0;
//						counter_target[i] = 0;
//						SR_found[i] = 0;
//						payload[i] = 0;
//						payload_counter[i] = 0; 
//						bandwidth_allocation[i] = 0;
//						SM_traffic_monitor[i] = 11;
//					}
//
//					break;
//			
//					}
//				}else if(bandwidth_allocation[i] > 0)
//					bandwidth_allocation[i]++;  
//			}
//
//		}
//
//	}
//