//---------------------------------------------------------------------------------------
//
//  DISTRIBUTED MEMPHIS  - version 5.0
//
//  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
//
//  Distribution:  September 2013
//
//  Source name:  queue.cpp
//
//  Brief description: Control process queue occupancy
//
//---------------------------------------------------------------------------------------

#include "queue.h"


// PROCESSO DE CONTROLE DA OCUPACAO DA FILA
// SE FILA HA ESPACOS LIVRES NA FILA
//   TEM_ESPACO_NA_FILA = TRUE
// DO CONTRARIO
//   TEM_ESPACO_NA_FILA = FALSE
void fila::in_proc_FSM(){
	sc_uint<4> local_first, local_last;
	
	local_first = first.read();
	local_last = last.read();
	
	if(reset_n.read() == false){
		tem_espaco_na_fila.write(true);
			credit_o.write(true);
	}
	else{
		if (((local_first==0) && (local_last==(BUFFER_TAM-2))) || (local_first==(local_last+2)) || (local_first==(local_last+1))){
			tem_espaco_na_fila.write(false);
			credit_o.write(false);
		}
		else if(((local_last - local_first) == 2) || ((local_first - local_last) == BUFFER_TAM-2)){
			tem_espaco_na_fila.write(true);
			credit_o.write(true);
		}
	}
}

// O ponteiro last � inicializado com o valor zero quando o reset � ativado.
// Quando o sinal rx � ativado indicando que existe um flit na porta de entrada �
// verificado se existe espa�o na fila para armazen�-lo. Se existir espa�o na fila o
// flit recebido � armazenado na posi��o apontada pelo ponteiro last e o mesmo �
// incrementado. Quando last atingir o tamanho da fila, ele recebe zero.
void fila::in_proc_updPtr(){
	if(reset_n.read()==false){
		last.write(0);
		for(int i=0;i<BUFFER_TAM;i++) buffer_in[i]=0;
	}
	else{
		if((tem_espaco_na_fila.read()==true) && (rx.read()==true)){
			buffer_in[last.read()] = data_in.read();
			//incrementa o last
			if(last.read()==(BUFFER_TAM - 1))
				last.write(0);
			else
				last.write((last.read() + 1));
		}
	}
}

// disponibiliza o dado para transmiss�o.
void fila::out_proc_data()
{

  data.write(buffer_in[first.read()]);

}

// Quando sinal reset � ativado a m�quina de estados avan�a para o estado S_INIT.
// No estado S_INIT os sinais h (que
// indica requisi��o de chaveamento) e data_av (que indica a exist�ncia de flit a ser
// transmitido) s�o inicializados com zero. Se existir algum flit na fila, ou seja, os
// ponteiros first e last apontarem para posi��es diferentes, a m�quina de estados avan�a
// para o estado S_HEADER.
// No estado S_HEADER � requisitado o chaveamento (h='1'), porque o flit na posi��o
// apontada pelo ponteiro first, quando a m�quina encontra-se nesse estado, � sempre o
// header do pacote. A m�quina permanece neste estado at� que receba a confirma��o do
// chaveamento (ack_h='1') ent�o o sinal h recebe o valor zero e a m�quina avan�a para
// S_SENDHEADER.
// Em S_SENDHEADER � indicado que existe um flit a ser transmitido (data_av='1'). A m�quina de
// estados permanece em S_SENDHEADER at� receber a confirma��o da transmiss�o (data_ack='1')
// ent�o o ponteiro first aponta para o segundo flit do pacote e avan�a para o estado S_PAYLOAD.
// Em S_END � indicado que o �ltimo flit deve ser transmitido (data_av='1') quando � recebida a
// confirma��o da transmiss�o (data_ack='1') a m�quina retorna ao estado S_INIT.

void fila::out_proc_FSM(){
	bool local_ack_h;
	bool local_data_ack;
	sc_uint<4> local_first;
	sc_uint<4> local_last;
	
	if(reset_n.read()==false){
		data_av.write(false);
		first.write(0);
		EA.write(S_INIT);
		h.write(false);
	}
	else{
		local_ack_h = ack_h.read();
		local_data_ack = data_ack.read();
		local_first = first.read();
		local_last = last.read();
				
		switch(EA.read()){
			case S_INIT:
				h.write(false);
				data_av.write(false);
				if(local_first != local_last){ // detectou dado na fila
					h.write(true);
					EA.write(S_HEADER);
				}
				else{
					EA.write(S_INIT);
				}
			break;
			
			case S_HEADER:
				if(local_ack_h==true){
					EA.write(S_SENDHEADER);      // depois de rotear envia o pacote
					h.write(false);
					data_av.write(true);
					sender.write(true);
				}
				else{
					EA.write(S_HEADER);
				}
			break;

			case S_SENDHEADER:
				if(local_data_ack==true){//confirma��o do envio do header
					//retira o header do buffer e se tem dado no buffer pede envio do mesmo
					if(local_first==(BUFFER_TAM-1)){
						first.write(0);
						if(local_last!=0)
							data_av.write(true);
						else
							data_av.write(false);
					}
					else{
						first.write(local_first + 1);
						if((local_first+1)!=local_last)
							data_av.write(true);
						else
							data_av.write(false);
					}
					EA.write(S_PAYLOAD);
				}
				else{
					EA.write(S_SENDHEADER);
				}
			break;
			  
			case S_PAYLOAD:
				if(local_data_ack==true && !buffer_in[first.read()].bit(32)){//confirma��o do envio de um dado que n�o � o tail
					//retira um dado do buffer e se tem dado no buffer pede envio do mesmo
					if(local_first == (BUFFER_TAM-1)){
						first.write(0);
						if(local_last!=0)
							data_av.write(true);
						else
							data_av.write(false);
					}
					else{
						first.write(local_first+1);
						if((local_first+1)!=local_last){
							data_av.write(true);
						}
						else{
							data_av.write(false);
						}
					}
					EA.write(S_PAYLOAD);
				}
				else{
					if(local_data_ack==true && buffer_in[first.read()].bit(32)){//confirma��o do envio do tail
						//retira um dado do buffer
						if(local_first==(BUFFER_TAM-1)){
							first.write(0);
						}
						else{
							first.write(local_first+1);
						}					
						data_av.write(false);
						sender.write(false);
						EA.write(S_END);
					}
					else{
						if(local_first!=local_last){//se tem dado a ser enviado faz a requisi��o
							data_av.write(true);
							EA.write(S_PAYLOAD);
						}
						else{
							EA.write(S_PAYLOAD);
						}
					}
				}
			break;	

			case S_END:
				data_av.write(false);
				EA.write(S_END2);
			break;
			
			case S_END2://estado necessario para permitir a libera��o da porta antes da solicita��o de novo envio
				data_av.write(false);
				EA.write(S_INIT);
			break;
		}
	}
}
