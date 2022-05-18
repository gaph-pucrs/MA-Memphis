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

void RouterCCwrapped::upd_rx(){
	int i;
	sc_uint<NPORT > l_rx_internal;

		for(i=0;i<NPORT;i++){
				//modified to add faults
				l_rx_internal[i] = rx[i].read();
	 }
	 rx_internal.write(l_rx_internal);
}

void RouterCCwrapped::upd_credit_i(){
	int i;
	sc_uint<NPORT > l_credit_i_internal;

	 for(i=0;i<NPORT;i++){
		//modified to add faults
		l_credit_i_internal[i] = credit_i[i].read();
	 }

	 credit_i_internal.write(l_credit_i_internal);
}


//output signals
//read from internal, write in output


void RouterCCwrapped::upd_credit_o(){
	int i;
	sc_uint<NPORT > l_credit_o_internal;

	l_credit_o_internal = credit_o_internal.read();

	for(i=0;i<NPORT;i++){
			//modified to add faults
			credit_o[i].write(l_credit_o_internal[i]);
	}
}

void RouterCCwrapped::upd_tx(){
	int i;
	sc_uint<NPORT > l_tx_internal;

	l_tx_internal = tx_internal.read();

	for(i=0;i<NPORT;i++){
		//modified to add faults
		tx[i].write(l_tx_internal[i]);
	}
}

void RouterCCwrapped::traffic_monitor(){
	int i;
	char aux[255];
	FILE *fp;

	if(reset.read() == 1){
		for(i = 0; i < NPORT; i++) {
			bandwidth_allocation[i] = 0;
			SM_traffic_monitor[i] = 0;
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


				switch(SM_traffic_monitor[i])
				{

					case 0: //Header
						SM_traffic_monitor[i] = 1;
						target_router[i] = data_in[i].read() & 0xFFFF;
						header_time[i] = (unsigned int)tick_counter.read();
						
						//printf("%d   --      %d\n",  (unsigned int)data_in[i].read(), target_router[i]); //debug
						break;

					case 1: //Payload
						payload[i] = data_in[i].read();
						SM_traffic_monitor[i] = 2;
						payload_counter[i] = data_in[i].read();
						break;					

					case 2: //Service
						service[i] = data_in[i].read();

						if(service[i] != 0x40 && service[i] != 0x70 && service[i] != 0x23 && service[i] != 0x00 && service[i] != 0x01 && service[i] != 0x31)
							SM_traffic_monitor[i] = 5;
						else
							SM_traffic_monitor[i] = 3;
						break;

					case 3: //If is task_allocation

						task_id[i] = data_in[i].read();
						if (service[i] == 0x00 || service[i] == 0x01 || service[i] == 0x31)
							SM_traffic_monitor[i] = 4;
						else
							SM_traffic_monitor[i] = 5;

						break;

					case 4:
						consumer_id[i] = data_in[i].read();
						SM_traffic_monitor[i] = 5;
						break;

					case 5:

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

							bandwidth_allocation[i] = 0;
							SM_traffic_monitor[i] = 0;
						}

					break;
				}
			} else if(bandwidth_allocation[i] > 0)
				bandwidth_allocation[i]++;	
		}
	}
}
