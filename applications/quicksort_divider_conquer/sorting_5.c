/*

                                (0)
                    (1)                     (2)
                (3)     (4)             *(5)     (6)
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
    int vector[FOURTH_VECTOR_SIZE];
    int half1[EIGHT_VECTOR_SIZE];
	int half2[EIGHT_VECTOR_SIZE];
    int i, j, k, aux, tamanho, halfTamanho;

    puts("Aguardando o vetor...\n");
    memphis_receive(&msg, sorting_2); // AQUI
    puts("Recebendo o vetor:\n");
    puts("--> Tamanho do vetor:\n");
    printf("%d\n", msg.length);
    puts("--> Vetor:\n");
    tamanho = msg.length; 
    for(i=0; i<tamanho; i++){
        vector[i] = msg.payload[i];
        printf("%d\n", vector[i]);
    }
    puts("Vetor recebido com sucesso!\n");

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
    memphis_send(&msg, sorting_11); // AQUI
    puts("Ramo direito:\n");
    for(i=0; i<halfTamanho; i++){
        msg.payload[i] = vector[aux];
        printf("%d\n", msg.payload[i]);
        aux++;
    }
    msg.length = halfTamanho;
    memphis_send(&msg, sorting_12); // AQUI

    // Aguarda o recebimento dos vetores organizados
    puts("Aguardando recebimento dos ramos:\n");
    puts("Ramo esquerdo:\n");
    memphis_receive(&msg, sorting_11); // AQUI
    for(i=0; i<msg.length; i++){
        half1[i] = msg.payload[i];
        printf("%d\n", msg.payload[i]);
    }
    puts("Ramo direito:\n");
    memphis_receive(&msg, sorting_12); // AQUI
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

    // Retorna o vetor para o pai
    puts("Retornando o vetor para a raiz:\n");
    msg.length = tamanho;
    memphis_send(&msg, sorting_2); // AQUI
    
	return 0;

}
