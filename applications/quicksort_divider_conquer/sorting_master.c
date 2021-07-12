/*

                                *(0)
                    (1)                     (2)
                (3)     (4)             (5)     (6)
              (7) (8) (9)(10)        (11)(12) (13)(14)

    Iaçanã Ianiski Weber
    - iacanaw@gmail.com
    06/2019
*/
#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "../quicksort_divider_conquer/sorting.h"

message_t msg;

int main(){
    int vector[VECTOR_SIZE];
    int half1[HALF_VECTOR_SIZE];
	int half2[HALF_VECTOR_SIZE];
    int i, j, k, aux, tamanho, halfTamanho;
    puts("Iniciando...\n");
    // Inicializa o vetor com valores decrescentes
    aux = VECTOR_SIZE;
    for(i = 0; i<VECTOR_SIZE; i++){
        vector[i] = aux;
        aux = aux - 1;
        printf("%d\n", vector[i]);
    }
    puts("Vetor Preenchido!\n");
    tamanho = VECTOR_SIZE;

    // Define o primeiro flit contendo o tamanho do vetor que está sendo enviado
	puts("Iniciando o envio das mensgaens de cada ramo...\n");
    halfTamanho = tamanho/2;
    aux = 0;
    puts("Ramo esquerdo:\n");
    for(i=0; i<halfTamanho; i++){
        msg.payload[i] = vector[aux];
        printf("%d\n", msg.payload[i]);
        aux++;
    }
    msg.length = halfTamanho;
    memphis_send(&msg, sorting_1);
    puts("Ramo direito:\n");
    for(i=0; i<halfTamanho; i++){
        msg.payload[i] = vector[aux];
        printf("%d\n", msg.payload[i]);
        aux++;
    }
    msg.length = halfTamanho;
    memphis_send(&msg, sorting_2);

    // Aguarda o recebimento dos vetores organizados
    puts("Aguardando recebimento dos ramos:\n");
    puts("Ramo esquerdo:\n");
    memphis_receive(&msg, sorting_1);
    for(i=0; i<msg.length; i++){
        half1[i] = msg.payload[i];
        printf("%d\n", msg.payload[i]);
    }
    puts("Ramo direito:\n");
    memphis_receive(&msg, sorting_2);
    for(i=0; i<msg.length; i++){
        half2[i] = msg.payload[i]; 
        printf("%d\n", msg.payload[i]);
    }

    puts("Realizando o merge:\n");
    // Realiza o merge dos vetores
    j = 0;
    k = 0;
    for(i=0; i<tamanho; i++){
        if(j == halfTamanho){
            vector[i] = half2[k];
            k++;
        }
        else if(k == halfTamanho){
            vector[i] = half1[j];
            j++;
        }
        else if(half1[j]<half2[k] ){
            vector[i] = half1[j];
            j++;
        }  
        else{
            vector[i] = half2[k];
            k++;
        }
        // Imprime o vetor organizado
        printf("%d\n", vector[i]);
        msg.payload[i] = vector[i];
    }
	return 0;

}
