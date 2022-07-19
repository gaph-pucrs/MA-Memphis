/*
autor: Geaninne Marchezan
contato: geaninne.mnl@gmail.com
*/
#include <stdio.h>
#include "multME.h"

message_t msg;

void receiveB(int *B){

	int i,j,sizeMSG;

	//Echo("Recebendo matriz B");	
	memphis_receive(&msg, master);

	int numMSG = msg.payload[0];

	sizeMSG= getSizeMSG(numMSG,N*N);
	for(i=0;i<numMSG;i++){

		if(i==0){


			for(j=1;j<msg.length;j++){
					B[j-1] = msg.payload[j];
				
			}

		
		}else if(i==(numMSG - 1)){
			
			sizeMSG = getSizeLastMSG(numMSG,sizeMSG,N*N);
			memphis_receive(&msg,master);
			for(j=0;j<msg.length;j++){

				B[(i* sizeMSG )  + j] = msg.payload[j];
			//	Echo(itoa(i* (sizeMSG ) -1 + j));
				//Echo(itoa(A[(i*sizeMSG) -3 + j]));

			
			}
		}else{			


			memphis_receive(&msg,master);
			//Echo(itoa(msg.length));
			for(j=0;j<msg.length;j++){
				B[(i * (msg.length)) -1 + j] = msg.payload[j];
			//	Echo(itoa((i * (msg.length)) + j));
			}
		
			sizeMSG = msg.length;
		}
	}


}

void sendR(int *R, int sizeR){

	int i, j;


    /*retorna o número de mensagens necessarias para enviar R*/
    int numMSG = getNumMSG(sizeR);	

    /*retorna o número de elementos que terá cada mensagem (exceto a ultima)*/
    int sizeMSG = getSizeMSG(numMSG,sizeR);

	for(j=0;j<numMSG;j++){

		/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
		if(j==(numMSG-1)){
				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,sizeR);
			//	Echo("ultima mensagem");
			//	Echo(itoa(sizeMSG));

		}
		for(i=0;i<sizeMSG;i++){
			msg.payload[i] = R[(j * sizeMSG) +i];
			//Echo(itoa(msg.payload[i]));
		}	

		
		msg.length = sizeMSG;

		memphis_send(&msg,master);
	}
}

void mult(int* A, int* B, int* R, int numLinhasA){
	int i,j,k;
	for(i=0; i<numLinhasA; i++){
		for(j=0; j<N;j++){
		 	for(k=0;k<N;k++){
		 		R[((i*N)+j)] += A[(i*N)+k] * B[(k*N)+j];
	
			}
		}
	}

}


 int main() {

 	printf("Inicio da aplicação slave2 %d\n", memphis_get_tick());
	
	int ini, fim, qtdElem, numLinhasA;
	int i,j,numMSG;

	int B [SIZE];
	puts("Recebendo B\n");
	receiveB(B);



	puts("Recebendo matriz A\n");
	memphis_receive(&msg,master);

	ini = msg.payload[0];
	fim = msg.payload[1];
	numMSG = msg.payload[2];
	qtdElem = msg.payload[3];
	int sizeMSG=0;
	int A[qtdElem];
	numLinhasA = fim - ini + 1;
	
	for(i=0;i<numMSG;i++){

		if(i==0){

			for(j=4;j<msg.length;j++){
				A[j-4] = msg.payload[j];
				//Echo(itoa(j-4));
			}


		}else if(i==(numMSG - 1)){
			

			memphis_receive(&msg,master);
			for(j=0;j<msg.length;j++){

				A[(i* sizeMSG ) - 3+ j] = msg.payload[j];
				//Echo(itoa(i* (sizeMSG ) - 3 + j));
				//Echo(itoa(A[(i*sizeMSG) -3 + j]));

			
			}
		}else{			


			memphis_receive(&msg,master);
			//Echo(itoa(msg.length));
			for(j=0;j<msg.length;j++){
				A[(i * (msg.length)) + j] = msg.payload[j];
			//	Echo(itoa((i * (msg.length)) - 3 + j));
			}
		
			sizeMSG = msg.length;
		}
	}

	/*Echo("matriz A");
	for(i=0;i<numLinhasA*N;i++){
		Echo(itoa(A[i]));
	}*/

	/*Echo("matriz B");
	for(i=0;i<N*N;i++){
		Echo(itoa(B[i]));
	}*/
	
	int tamMatR = numLinhasA*N;
	int R[tamMatR];
	//Echo("tamanho da matriz R");
	//Echo(itoa(tamMatR));
	/*Echo("Multiplicacao");
	Echo(itoa(GetTick()));*/

	puts("Multiplicando Matrizes\n");
	mult(A,B,R, numLinhasA);
	
	//Echo("Enviando resultado");
	//Echo(itoa(GetTick()));
	sendR(R,tamMatR);
	
	printf("Fim da aplicação slave2 %d\n", memphis_get_tick());

	return 0;
}	
