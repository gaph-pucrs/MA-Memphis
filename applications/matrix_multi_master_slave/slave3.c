#include "../matrix_multi_master_slave/multME.h"

Message msg;

void enviaMaster(int numMSG, int d,int tamMatR, int *matrizR){
int i, j,k,auxd,auxd2;
	for(j=0;j<numMSG;j++){
	
			if(j==0){
			
				msg.msg[0] = numMSG;
			
				for(k=0;k<d-1;k++){
				  
				  msg.msg[k+1] = matrizR[k];
				  
				}
			}else if(j==1){
				//Echo("if2");
			
				for(k=0;k<d;k++){
			 		msg.msg[k]=matrizR[((d-1)+k)];

					//Echo(itoa(msg.msg[k]));
				}

			}else if((j > 1) && (j < (numMSG-1))){
			//Echo("if3");
			//Echo(itoa(j));
				for(k=0;k<d;k++){
			 		msg.msg[k]=matrizR[((j*d)+k-1)];

					//Echo(itoa(msg.msg[k]));
				}

			}else if(j==(numMSG-1)){
			//Echo("else");
				 auxd = d*numMSG;
				//Echo("antesdo if");
				if(auxd>tamMatR){
				//Echo("entrou if");
					auxd2=auxd-tamMatR;
					for(k=0;k<(d-auxd2)+1;k++){
			 			msg.msg[k]=matrizR[((j*d)+k-1)];

						//Echo(itoa(msg.msg[k]));
					}
				}


			}
		msg.length = d;

		Send(&msg,master);

	}
}

void receiveB(int *B){

	int i,j,k;

	Echo("Recebendo matriz B");	
	Receive(&msg, master);

	int numMSG = msg.msg[0];

	for(i=0;i<numMSG;i++){

		if(i==0){
			for(j=1;j<msg.length;j++){
				B[j-1] = msg.msg[j];
			}
		}else{
			Receive(&msg,master);
			for(j=0;j<msg.length;j++){

				B[(i * (msg.length - 1)) + j] = msg.msg[j];
			}

		}

	}
}

void sendR(int *R, int sizeR){

	int i, j, aux, aux2;


    /*retorna o número de mensagens necessarias para enviar R*/
    int numMSG = getNumMSG(sizeR);	

    /*retorna o número de elementos que terá cada mensagem (exceto a ultima)*/
    int sizeMSG = getSizeMSG(numMSG,sizeR);

	for(j=0;j<numMSG;j++){

		/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
		if(sizeLastMSG(j,numMSG,sizeMSG,sizeR)){
				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,sizeR);

		}
		for(i=0;i<sizeMSG;i++){
			msg.msg[i] = R[(j * sizeMSG) +i];
		}	

		
		msg.length = sizeMSG;

		Send(&msg,master);
	}
}



 int main() {

 	Echo("Inicio da aplicacao task1");
	Echo(itoa(GetTick()));
	
	int ini, fim, qtdElem, numLinhasA,k;
	int i,j,numMSG,sizeMSG;

	int B [MATRIX_SIZE];

	int tamMSG,auxd2,auxd;
	 	
	receiveB(B);

	Echo("Recebendo matriz A");	
	Receive(&msg,master);

	ini = msg.msg[0];
	fim = msg.msg[1];
	numMSG = msg.msg[2];
	qtdElem = msg.msg[3];
	int A[qtdElem];
	numLinhasA = fim - ini + 1;
	
	for(i=0;i<numMSG;i++){

		if(i==0){

			for(j=1;j<msg.length;j++){
				A[j-1] = msg.msg[j];
			}
		}else{

			Receive(&msg,master);
			for(j=0;j<msg.length;j++){
				A[(i * (msg.length - 1)) + j] = msg.msg[j];
			}

		}
	}
	
	Echo(itoa(GetTick()));
	int tamMatR = numLinhasA*N;
	int R[tamMatR];
	Echo("Multiplicacao");
	Echo(itoa(GetTick()));

	for(i=0; i<numLinhasA; i++){
		for(j=0; j<N;j++){
		 	for(k=0;k<N;k++){
		 		R[((i*N)+j)] += A[(i*N)+k] * B[(k*N)+j];
	
			}
								//Echo(itoa(matrizR1[((i*N)+j)]));
		}
	}

	Echo("Enviando resultado");
	Echo(itoa(GetTick()));
	sendR(R,tamMatR);
	
	Echo("Fim da aplicação task1");
	Echo(itoa(GetTick()));

	exit();
}	
