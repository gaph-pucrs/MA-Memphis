#include <api.h>
#include <stdlib.h>

#include "../matrix_multi_master_slave/multME.h"

Message msg;

/*vetores que armazenam: linha inicial, linha final e quantidade de elementos para cada task*/
int ini[numTasks], fim[numTasks],qtdElem[numTasks];


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

void adjust( int *ini,  int *fim, int *qtdElem){
       	int i, chunk, chunkPlus, rest;

    /*verifica se a quantidade de linhas será igual para cada tarefa*/
	if(N%numTasks==0){
		chunk = N/numTasks;
		for(i=0;i<numTasks;i++){

			/*calcula a linha inicial da tarefa i*/
			ini[i] = i*chunk;

			/*calcula a linha final da tarefa i*/
			fim[i] = (i * chunkPlus) + chunk;
			//fim[i] = (i*chunk)+(chunk-1);

			/*calcula a quantidade de elementos que serão processados pela tarefa i*/
			qtdElem[i] = ((fim[i] - ini[i])+1)*N;
		}
		
	}else{
		chunk = N/numTasks;
		chunkPlus = chunk+1;
		rest = N%numTasks;
		for(i=0;i<numTasks;i++){
			if(i < rest){
				ini[i] = i*chunkPlus;
				fim[i] = (i*chunkPlus)+chunk;
				qtdElem[i] = ((fim[i] - ini[i])+1)*N;
			}else{
				ini[i] = ((fim[i-1])+1);
				fim[i] = ini[i]+ chunk - 1;
				qtdElem[i] = ((fim[i] - ini[i]) + 1) * N;
			}
		}                 
	}
}

void receiveR(int *R){
	int i,j,k, numMSG,sizeMSG, aux;

	for(i=0;i<numTasks;i++){
	/*calcula o numero de mesagens necessárias*/
		numMSG = getNumMSG(qtdElem[i]);
		sizeMSG = getSizeMSG(numMSG,qtdElem[i]);

		for(j=0;j<numMSG;j++){

		/*recebe mensagem*/
			Receive(&msg,task[i]);

		/*verifica se está na ultima mensagem e o tamanho da mesma*/

			if(sizeLastMSG(j,numMSG,sizeMSG,qtdElem[i])){

				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,qtdElem[i]);


			}

			for(k=0;k<sizeMSG;k++){
				R[(j * sizeMSG) + (ini[i] * N) + k] = msg.msg[k];

			}

		}
	}
}
void sendB(int *B){

	int i, j, aux, aux2;


    /*retorna o número de mensagens necessarias para enviar B*/
	
    int numMSG = getMasterNumMSG();	

    /*retorna o número de elementos que terá cada mensagem (exceto a ultima)*/
    int sizeMSG = getSizeMSG(numMSG,MATRIX_SIZE + 1);

	/*armazena o numero de mensagem a serem enviadas na primeira posição da primeira mensagem*/
	msg.msg[0] = numMSG;

	for(j=0;j<numMSG;j++){

		/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
		if(sizeLastMSG(j,numMSG,sizeMSG,MATRIX_SIZE +1)){
				sizeMSG = getSizeLastMSG(numMSG,sizeMSG,qtdElem[i]);

		}
		if(j==0){

			for(i=1;i<sizeMSG;i++){
					msg.msg[i] = B[i-1];
			}
		}else{

			for(i=0;i<sizeMSG;i++){
					msg.msg[i] = B[(j * sizeMSG) +i -1];
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

	int i,j,k, numMSG,sizeMSG,aux;
	
	/*calcula distribuição de carga*/
	adjust(ini, fim, qtdElem);


	for(i=0;i<numTasks;i++){

		numMSG = getNumMSG(qtdElem[i]+4);
		sizeMSG = getSizeMSG(numMSG, qtdElem[i]+4);

				
		for(j=0;j<numMSG;j++){

			/*verifica se está na ultima mensagem e recalcula o tamannho da mesma*/
			if(sizeLastMSG(j,numMSG,sizeMSG,qtdElem[i] + 4)){
				aux = sizeMSG * numMSG;
				sizeMSG = sizeMSG - (aux - (qtdElem[i] + 4)); 
			}

			if(j==0){

				msg.msg[0] = ini[i];
				msg.msg[1] = fim[i];
				msg.msg[2] = numMSG;
				msg.msg[3] = qtdElem[i];

				for(k=4;k<sizeMSG;k++){
			 		msg.msg[k]= A[(k - 4 +(ini[i]*N))];
			 		//Echo(itoa(msg.msg[k+4]));
				}

			}else{

				for(k=0;k<sizeMSG;k++){
			 		msg.msg[k]=A[(((sizeMSG*j)+ k - 4 + (ini[i]*N)))];

				}
			}

			msg.length = sizeMSG;

			Send(&msg, task[i]);

		}
	
	}
}
		
	
 int main(){
     

	Echo("Inicio da aplicacao start");
	Echo(itoa(GetTick()));
	Echo(itoa(N));

	int i,j,k, aux, aux2;

	/*matrizes de entrada*/
	int A [MATRIX_SIZE];
	int B[MATRIX_SIZE];

	/*matriz resultado*/
	int R[MATRIX_SIZE];


    /*preenche matrizes*/    
	fillMatrix(A, B);
	
	/*envia matriz B*/
	sendB(B);

	/*calcula a distribuição de carga e envia matriz A*/
	sendA(A);

	/*recebe resultado e constroi matriz resultante*/
	receiveR(R);

	
	Echo("Fim da aplicação");
	Echo(itoa(GetTick()));
	
	exit();
}


