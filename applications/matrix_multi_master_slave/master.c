/*
autor: Geaninne Marchezan
contato: geaninne.mnl@gmail.com
*/

#include <api.h>
#include <stdlib.h>
#include "multME.h"

Message msg;

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
			Receive(&msg,task[i]);

		/*verifica se está na ultima mensagem e o tamanho da mesma*/

			if(j==(numMSG - 1)){

				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,qtdElemA[i]);


			}

			for(k=0;k<sizeMSG;k++){
				//Echo(itoa(msg.msg[k]));
				R[(j * sizeAnt) + (ini[i] * N) + k] = msg.msg[k];
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
	msg.msg[0] = numMSG;

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
					msg.msg[i] = B[i-1];
					//Echo(itoa(i-1));
			}
		}else{

			for(i=0;i<sizeMSG;i++){
					msg.msg[i] = B[(j * msg.length) +i -1];
					//Echo(itoa((j * msg.length) +i -1));
			}	

		
		}

		msg.length = sizeMSG;

		/*envia mensagem para as tasks*/
		for(i=0;i<numTasks;i++){
			Send(&msg,task[i]);
		}
	}
}

void sendA( int *A){
	Echo("Enviando A");
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

				msg.msg[0] = ini[i];
				msg.msg[1] = fim[i];
				msg.msg[2] = numMSG;
				msg.msg[3] = qtdElemA[i];

				for(k=4;k<sizeMSG;k++){
			 		msg.msg[k]= A[(k - 4 +(ini[i]*N))];
			 		//Echo(itoa(msg.msg[k]));
				}

			}else{

				for(k=0;k<sizeMSG;k++){
			 		msg.msg[k]=A[(((sizeMSG*j)+ k - 4 + (ini[i]*N)))];
			 		//Echo(itoa(msg.msg[k]));


				}
			}

			msg.length = sizeMSG;
		//	Echo(itoa(msg.length));
			Send(&msg, task[i]);

		}
	
	}
}
		
	
 int main(){
     

	Echo("Inicio da aplicacao master");
	Echo(itoa(GetTick()));
	Echo(itoa(N));
	/*Echo("SIZE");
	Echo(itoa(SIZE));*/
	int i;

	/*matrizes de entrada*/
	int A[SIZE];
	int B[SIZE];

	/*matriz resultado*/
	int R[SIZE];

	Echo("Preenchendo matrizes");
    /*preenche matrizes*/    
	fillMatrix(A, B);

/*	Echo("B");
	for(i=0;i<SIZE;i++){
		Echo(itoa(B[i]));
	}*/
	
	/*envia matriz B*/

	Echo("Enviando B");
	sendB(B);


	/*calcula distribuição de carga*/
	adjust();

	/*envia matriz A*/
	sendA(A);

	
	Echo("Esperando R");
	/*recebe resultado e constroi matriz resultante*/
	receiveR(R);

	Echo("matriz resultante");
	for(i=0;i<SIZE;i++){
		Echo(itoa(R[i]));
	}

	
	Echo("Fim da aplicação master");
	Echo(itoa(GetTick()));
	
	/*for(i=0;i<SIZE;i++){
		Echo(itoa(R[i]));
	}*/
		exit();
}


