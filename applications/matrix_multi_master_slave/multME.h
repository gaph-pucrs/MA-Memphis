#include <stdlib.h>
#include <stdio.h>
#include <api.h>
#define N 30
#define MATRIX_SIZE N*N
#define numTasks 3
int task[numTasks] = {slave1,slave2,slave3};
//void preencheMatriz(int *matrizA, int *matrizB); 
//void printaMatriz(int *matrizResult);
//void multiplica(int *matriz1, int *matriz2, int *matrizResult, int iniP, int fimP);
//int getNumMSG(int qtdElem);
//int getSizeMSG(int numMSG,  int qtdElem);


int getSizeMSG(int numMSG,  int qtdElem){
	int sizeMSG;
	if((qtdElem%numMSG) ==0){
		return sizeMSG = qtdElem/numMSG;
		
	}else{
		return sizeMSG = (qtdElem/numMSG)+1;
		
		
	}
	
}

int sizeLastMSG(int i, int numMSG, int sizeMSG, int qtdElem){

	if((i==(numMSG -1)) && ((sizeMSG * numMSG) > qtdElem)){
		return 1;
	} else{

		return 0;
	}

}

int getNumMSG( int qtdElem){
	int numMSG;
	if (((qtdElem)%128==0)){
		numMSG = (qtdElem/MSG_SIZE);
		return numMSG;
	}else{
		numMSG = ((qtdElem/MSG_SIZE)+1);
		return numMSG;
	}
	
}	

int getSizeLastMSG(int numMSG, int sizeMSG, int qtdElem){
	int aux, sizeLastMSG;	

	aux = sizeMSG * numMSG;
	sizeLastMSG = sizeMSG - (aux - qtdElem);
	return sizeLastMSG;

}
int getMasterNumMSG(){
	int numMSG;
	if (((MATRIX_SIZE + 1)%128==0)){
		numMSG = ((MATRIX_SIZE + 1)/MSG_SIZE);
		return numMSG;
	}else{
		numMSG = (((MATRIX_SIZE + 1)/MSG_SIZE)+1);
		return numMSG;
	}
	
}	



/*
void multiplica(int *matriz1, int *matriz2, int *matrizResult, int iniP, int fimP){
	int  i, j, k,w;
	//int iniP, fimP;
	//iniP = posicao[0];
	//fimP = posicao[1];
	Echo("teste");
	Echo(itoa(iniP));
	Echo(itoa(fimP));
	for(i=0;i<10;i++){
	
		for(j=0; j<N;j++){
		 	for(k=0;k<N;k++){
		 		//matrizResult[((i*N)+j)] += matrizA[(i*N)+k] * matrizB[(k*N)+j];
							
			}

		}	
	 }
}*/
