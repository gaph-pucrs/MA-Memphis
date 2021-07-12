/*
autor: Geaninne Marchezan
contato: geaninne.mnl@gmail.com
*/

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "multME.h"

message_t msg;

/*vetores que armazenam: linha inicial, linha final e quantidade de elementos para cada task*/
int ini[numTasks], fim[numTasks],qtdElemA[numTasks];


/*Função que preenche matrizes  A e B*/

void fillMatrix( int *A, int *B){
	int i,j;

	for(i=0; i<N; i++){
		for(j=0; j<N; j++){
			
		    A[(i*N)+j] = i;
			B[(i*N)+j]= i+j;
		}
	}

}

/*Função que faz a distribuição de carga entre as tarefas escravas*/

void adjust(){
       	int i, chunk, chunkPlus, rest;

    /*verifica se a quantidade de linhas será igual para cada tarefa*/
	if(N%numTasks==0){
		chunk = N/numTasks;
		for(i=0;i<numTasks;i++){

			/*calcula a linha inicial da tarefa i*/
			ini[i] = i*chunk;

			/*calcula a linha final da tarefa i*/
			fim[i] = (i * chunk) + chunk -1;
			//fim[i] = (i*chunk)+(chunk-1);

			/*calcula a quantidade de elementos que serão processados pela tarefa i*/
			qtdElemA[i] = ((fim[i] - ini[i]) +1)*N;
		}
		
	}else{
		chunk = N/numTasks;
		chunkPlus = chunk+1;
		rest = N%numTasks;
		for(i=0;i<numTasks;i++){
			if(i < rest){
				ini[i] = i*chunkPlus;
				fim[i] = (i*chunkPlus)+chunk;
				qtdElemA[i] = ((fim[i] - ini[i])+1)*N;
			}else{
				ini[i] = ((fim[i-1])+1);
				fim[i] = ini[i]+ chunk - 1;
				qtdElemA[i] = ((fim[i] - ini[i]) + 1) * N;
			}
		}                 
	}
}

void receiveR(int *R){
	int i,j,k, numMSG,sizeMSG,sizeAnt;

	for(i=0;i<numTasks;i++){
	/*calcula o numero de mesagens necessárias*/
		numMSG = getNumMSG(qtdElemA[i]);
		sizeMSG = getSizeMSG(numMSG,qtdElemA[i]);
		sizeAnt = sizeMSG;

		for(j=0;j<numMSG;j++){

		/*recebe mensagem*/
			memphis_receive(&msg,task[i]);

		/*verifica se está na ultima mensagem e o tamanho da mesma*/

			if(j==(numMSG - 1)){

				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,qtdElemA[i]);


			}

			for(k=0;k<sizeMSG;k++){
				//Echo(itoa(msg.payload[k]));
				R[(j * sizeAnt) + (ini[i] * N) + k] = msg.payload[k];
			}
			sizeAnt = sizeMSG;

		}
	}
}
void sendB(int *B){

	int i, j, aux;

	int qtdElem = (N*N) + 1;
    /*retorna o número de mensagens necessarias para enviar B*/
    int numMSG = getNumMSG(qtdElem);	
    /*Echo("numMSG");
    Echo(itoa(numMSG));*/

    /*retorna o número de elementos que terá cada mensagem (exceto a ultima)*/
    int sizeMSG = getSizeMSG(numMSG,qtdElem);
    /*Echo("sizeMSG");
    Echo(itoa(sizeMSG));*/

	/*armazena o numero de mensagem a serem enviadas na primeira posição da primeira mensagem*/
	msg.payload[0] = numMSG;

	for(j=0;j<numMSG;j++){

		/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
		if(j==(numMSG-1)){
				aux = sizeMSG * numMSG;
				sizeMSG = sizeMSG - (aux - (qtdElem )); 
			/*	Echo("ultima mensagem");
				Echo(itoa(sizeMSG));*/
			}
		if(j==0){

			for(i=1;i<sizeMSG;i++){
					msg.payload[i] = B[i-1];
					//Echo(itoa(i-1));
			}
		}else{

			for(i=0;i<sizeMSG;i++){
					msg.payload[i] = B[(j * msg.length) +i -1];
					//Echo(itoa((j * msg.length) +i -1));
			}	

		
		}

		msg.length = sizeMSG;

		/*envia mensagem para as tasks*/
		for(i=0;i<numTasks;i++){
			memphis_send(&msg,task[i]);
		}
	}
}

void sendA( int *A){
	puts("Enviando A\n");
	int i,j,k, numMSG,sizeMSG,aux;
	
	int tamanho;

	for(i=0;i<numTasks;i++){
		/*Echo("qtdELEMA");
		Echo(itoa(qtdElemA[i]));*/
		tamanho = qtdElemA[i] + 4;
		numMSG = getNumMSG(tamanho);
		/*Echo("numMSG");
		Echo(itoa(numMSG));
		Echo("tamanho");
		Echo(itoa(tamanho));*/
		sizeMSG = getSizeMSG(numMSG, tamanho);
		/*Echo("sizeMSG");
		Echo(itoa(sizeMSG));*/
				
		for(j=0;j<numMSG;j++){

			/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
			if(j==(numMSG-1)){
				aux = sizeMSG * numMSG;
				sizeMSG = sizeMSG - (aux - (tamanho )); 
				/*Echo("ultima mensagem");
				Echo(itoa(sizeMSG));*/
			}

			if(j==0){

				msg.payload[0] = ini[i];
				msg.payload[1] = fim[i];
				msg.payload[2] = numMSG;
				msg.payload[3] = qtdElemA[i];

				for(k=4;k<sizeMSG;k++){
			 		msg.payload[k]= A[(k - 4 +(ini[i]*N))];
			 		//Echo(itoa(msg.payload[k]));
				}

			}else{

				for(k=0;k<sizeMSG;k++){
			 		msg.payload[k]=A[(((sizeMSG*j)+ k - 4 + (ini[i]*N)))];
			 		//Echo(itoa(msg.payload[k]));


				}
			}

			msg.length = sizeMSG;
		//	Echo(itoa(msg.length));
			memphis_send(&msg, task[i]);

		}
	
	}
}
		
	
 int main(){
     

	printf("Inicio da aplicacao master %d\n", memphis_get_tick());
	/*Echo("SIZE");
	Echo(itoa(SIZE));*/
	int i;

	/*matrizes de entrada*/
	int A[SIZE];
	int B[SIZE];

	/*matriz resultado*/
	int R[SIZE];

	puts("Preenchendo matrizes\n");
    /*preenche matrizes*/    
	fillMatrix(A, B);

/*	Echo("B");
	for(i=0;i<SIZE;i++){
		Echo(itoa(B[i]));
	}*/
	
	/*envia matriz B*/

	puts("Enviando B\n");
	sendB(B);


	/*calcula distribuição de carga*/
	adjust();

	/*envia matriz A*/
	sendA(A);

	
	puts("Esperando R\n");
	/*recebe resultado e constroi matriz resultante*/
	receiveR(R);

	puts("matriz resultante\n");
	for(i=0;i<SIZE;i++){
		printf("%d ", R[i]);
	}

	
	printf("Fim da aplicação master %d\n", memphis_get_tick());
	
	/*for(i=0;i<SIZE;i++){
		Echo(itoa(R[i]));
	}*/
		return 0;
}


