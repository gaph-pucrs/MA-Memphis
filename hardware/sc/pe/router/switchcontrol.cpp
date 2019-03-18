//---------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS  - version 5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  switchcontrol.cpp
//
//  Brief description: Control of the states of the NoC, and executions for each state.
//
//---------------------------------------------------------------------------------------

#include "switchcontrol.h"

void switch_control::controle_comb(){
	regquartoflit lx_local,ly_local,tx_local,ty_local;
	
	
	lx_local=lx.read();
	ly_local=ly.read();
	tx_local=tx.read();
	ty_local=ty.read();
		
	switch(EA.read()){
		case S0:
			PE.write(S1);
		break;
		case S1:
			if(ask.read()==1){
				PE.write(S2);
			}
			else{
				PE.write(S1);
			}
		break;
		case S2:
			PE.write(S3);
		break;
		case S3:
			//----------- begin of special routing algorithm to a external component -------
			if(lx_local == tx_local && ly_local == ty_local && io_dir.read().bit(2) == 1){
				if (io_dir.read().bit(1) == 0 && free[dirx.read()].read()==1){
					PE.write(S5);
				} else if (io_dir.read().bit(1) == 1 && free[diry.read()].read()==1){
					PE.write(S6);
				} else {
					PE.write(S1);
				}
			}
			//----------- end of special routing algorithm to a external component -------
			else if(lx_local == tx_local && ly_local == ty_local && free[LOCAL].read()==1){
				PE.write(S4);
			}
			else{
				if(lx_local!=tx_local && free[dirx.read()].read()==1){
					PE.write(S5);
				}
				else{
					if( lx_local == tx_local && ly_local != ty_local && free[diry.read()].read()==1){
						PE.write(S6);
					}
					else{
						PE.write(S1);
					}
				}
			}
		break;
		case S4:
			PE.write(S7);
		break;
		case S5:
			PE.write(S7);
		break;
		case S6:
			PE.write(S7);
		break;
		case S7:
			PE.write(S1);
		break;
	}
}

void switch_control::state_sequ(){
	if(reset.read()==0){
		EA.write(S0);
	}
	else{
		EA.write(PE.read());
	}
}

		
void switch_control::controle_sequ(){
	reg_mux mux_in_local, mux_out_local;
	
		mux_out_local=mux_out.read();
		mux_in_local=mux_in.read();

		sender_ant[LOCAL].write(sender[LOCAL].read());
		sender_ant[EAST].write(sender[EAST].read());
		sender_ant[WEST].write(sender[WEST].read());
		sender_ant[NORTH].write(sender[NORTH].read());
		sender_ant[SOUTH].write(sender[SOUTH].read());

		switch(EA.read()){
			case S0://Zera variáveis
				sel.write(0);
				
				ack_h[LOCAL].write(0);
				ack_h[EAST].write(0);
				ack_h[WEST].write(0);
				ack_h[NORTH].write(0);
				ack_h[SOUTH].write(0);
				
				free[LOCAL].write(1);
				free[EAST].write(1);
				free[WEST].write(1);
				free[NORTH].write(1);
				free[SOUTH].write(1);
				
				sender_ant[LOCAL].write(0);
				sender_ant[EAST].write(0);
				sender_ant[WEST].write(0);
				sender_ant[NORTH].write(0);
				sender_ant[SOUTH].write(0);
				
				mux_out.write(0);
				mux_in.write(0);
			break;
			case S1://Chegou um header
				ack_h[LOCAL].write(0);
				ack_h[EAST].write(0);
				ack_h[WEST].write(0);
				ack_h[NORTH].write(0);
				ack_h[SOUTH].write(0);
			break;
			case S2://Seleciona quem tera direito a requisitar roteamento
				sel.write(prox.read());
			break;
			case S4://Estabelece a conexão com a porta LOCAL
				mux_in_local.range(sel.read()*3+2,sel.read()*3)=LOCAL;
				mux_in.write(mux_in_local);
				
				mux_out_local.range(LOCAL*3+2,LOCAL*3)=sel;
				mux_out.write(mux_out_local);
				
				free[LOCAL].write(0);
				
				ack_h[sel.read()].write(1);
				number_pck++;

			break;
			case S5://Estabelece a conexão com a porta EAST ou WEST
				mux_in_local.range(sel.read()*3+2,sel.read()*3)=dirx.read();
				mux_in.write(mux_in_local);
				
				mux_out_local.range(dirx.read()*3+2,dirx.read()*3)=sel;
				mux_out.write(mux_out_local);
				
				free[dirx.read()].write(0);
				
				ack_h[sel.read()].write(1);
			break;
			case S6://Estabelece a conexão com a porta NORTH ou SOUTH
				mux_in_local.range(sel.read()*3+2,sel.read()*3)=diry.read();
				mux_in.write(mux_in_local);
				
				mux_out_local.range(diry.read()*3+2,diry.read()*3)=sel;
				mux_out.write(mux_out_local);
				
				free[diry.read()].write(0);
				
				ack_h[sel.read()].write(1);
			break;
			default:
				ack_h[sel.read()].write(0);
			break;
		}
		
		if((sender[LOCAL].read()==0)&&(sender_ant[LOCAL].read()==1)){
			free[mux_in_local.range(LOCAL*3+2,LOCAL*3)].write(1);
		}
		if((sender[EAST].read()==0)&&(sender_ant[EAST].read()==1)){
			free[mux_in_local.range(EAST*3+2,EAST*3)].write(1);
		}
		if((sender[WEST].read()==0)&&(sender_ant[WEST].read()==1)){
			free[mux_in_local.range(WEST*3+2,WEST*3)].write(1);
		}
		if((sender[NORTH].read()==0)&&(sender_ant[NORTH].read()==1)){
			free[mux_in_local.range(NORTH*3+2,NORTH*3)].write(1);
		}
		if((sender[SOUTH].read()==0)&&(sender_ant[SOUTH].read()==1)){
			free[mux_in_local.range(SOUTH*3+2,SOUTH*3)].write(1);
		}
}

void switch_control::arbitro_comb(){
	regmetadeflit header_local;
	regquartoflit lx_local,ly_local,tx_local,ty_local;
	sc_uint<3> io_dir_local;
			
	if(h[LOCAL].read()==1 || h[EAST].read()==1 || h[WEST].read()==1 || h[NORTH].read()==1 || h[SOUTH].read()==1){
		ask.write(1);
	}
	else{
		ask.write(0);
	}
	

	switch(sel.read()){
		case LOCAL:
				if     (h[EAST ].read()==1) prox.write(EAST);
				else if(h[WEST ].read()==1) prox.write(WEST);
				else if(h[NORTH].read()==1) prox.write(NORTH);
				else if(h[SOUTH].read()==1) prox.write(SOUTH);
				else                 prox.write(LOCAL);
				
				header_local = data[LOCAL].read();
				io_dir_local = data[LOCAL].read().range(TAM_FLIT-1, TAM_FLIT-3);
		break;
		case EAST:
				if     (h[WEST ].read()==1) prox.write(WEST);
				else if(h[NORTH].read()==1) prox.write(NORTH);
				else if(h[SOUTH].read()==1) prox.write(SOUTH);
				else if(h[LOCAL].read()==1) prox.write(LOCAL);
				else                 prox.write(EAST);
				
				header_local = data[EAST].read();
				io_dir_local = data[EAST].read().range(TAM_FLIT-1, TAM_FLIT-3);
		break;
		case WEST:
				if     (h[NORTH].read()==1) prox.write(NORTH);
				else if(h[SOUTH].read()==1) prox.write(SOUTH);
				else if(h[LOCAL].read()==1) prox.write(LOCAL);
				else if(h[EAST ].read()==1) prox.write(EAST);
				else                 prox.write(WEST);
				
				header_local = data[WEST].read();
				io_dir_local = data[WEST].read().range(TAM_FLIT-1, TAM_FLIT-3);
		break;
		case NORTH:
				if     (h[SOUTH].read()==1) prox.write(SOUTH);
				else if(h[LOCAL].read()==1) prox.write(LOCAL);
				else if(h[EAST ].read()==1) prox.write(EAST);
				else if(h[WEST ].read()==1) prox.write(WEST);
				else                 prox.write(NORTH);
				
				header_local = data[NORTH].read();
				io_dir_local = data[NORTH].read().range(TAM_FLIT-1, TAM_FLIT-3);
		break;
		case SOUTH:
				if     (h[LOCAL].read()==1) prox.write(LOCAL);
				else if(h[EAST ].read()==1) prox.write(EAST);
				else if(h[WEST ].read()==1) prox.write(WEST);
				else if(h[NORTH].read()==1) prox.write(NORTH);
				else                 prox.write(SOUTH);
				header_local = data[SOUTH].read();
				io_dir_local = data[SOUTH].read().range(TAM_FLIT-1, TAM_FLIT-3);
		break;
	}
		
	lx_local=address.range((METADEFLIT-1),QUARTOFLIT);
	ly_local=address.range((QUARTOFLIT-1),0);
	
	lx.write(lx_local);
	ly.write(ly_local);

	tx_local=header_local.range((METADEFLIT-1),QUARTOFLIT);
	ty_local=header_local.range((QUARTOFLIT-1),0);
	
	tx.write(tx_local);
	ty.write(ty_local);
	
	io_dir.write(io_dir_local);

	//if(lx_local > tx_local){ //Old dirx selection, commented due the IO routing implementation
	if( (lx_local > tx_local) || (io_dir_local.bit(2)==1 && io_dir_local.bit(1)==0 && io_dir_local.bit(0)==1)){
		dirx.write(WEST);
	}
	else{
		dirx.write(EAST);
	}
	
	//if(ly_local < ty_local){//Old diry selection, commented due the IO routing implementation
	if( (ly_local < ty_local) || (io_dir_local.bit(2)==1 && io_dir_local.bit(1)==1 && io_dir_local.bit(0)==0)){
		diry.write(NORTH);
	}
	else{
		diry.write(SOUTH);
	}


}
