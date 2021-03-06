/* ARTUR HENRIQUE BRANDÃO DE SOUZA
	150118783
	Corrida = O objetivo é que existam vários corredores em que estes estão posições até que chegue em um momento em que acabará a
a corrida e será verificado as posições dos carros participando. Assim, como terão apenas 3 bombas de gasolina, caso acabe no carro, 
estes terão de ficar em uma fila esperando para encher o tanque até que volte. Obs: Não necessariamente será enchido o tanque por 
completo, pode ser que seja enchido menos para que volte mais rápido para a corrida. Assim, ao um carro entrar no posto, quem term 
prioridade para pegar as posições serão os carros logo atrás destas.

*/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define QNT_CARROS 10
#define QNT_BOMBAS_GASOLINA 1
#define QNT_FRENTISTAS 1
#define TEMPO_TOTAL 50


typedef struct{
	int posicao;
	int ocupado;
	int carro_ocupado;

}posicoes_carros;

posicoes_carros posicoes[QNT_CARROS];

pthread_cond_t carro_status = PTHREAD_COND_INITIALIZER;
pthread_cond_t carro_status2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t frentista_status = PTHREAD_COND_INITIALIZER;
pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gasolina_lock = PTHREAD_MUTEX_INITIALIZER;
int carros_pista, fim = 0, enchendo_gasolina=0; /* Ao chegar em um, significa que acabará a corrida*/
int bombas_gasolina = QNT_BOMBAS_GASOLINA;
int  bomba_cheia=0;
int bombeiro_car = 0;

int tempo_durante ;
int tempo_momentaneo;
int tempo_inicio;


void * print_posicoes(void *arg){
	int i,j=0, aux, achou =0;
	while(fim ==0){
		sleep(4);
		tempo_durante = TEMPO_TOTAL-( tempo_momentaneo - tempo_inicio);
		pthread_mutex_lock(&turno);
		pthread_mutex_lock(&lock);
		pthread_mutex_lock(&gasolina_lock);

		// for(i=0;i<QNT_CARROS;i++){ /*Organiza o vetor para ser printado na ordem que estão os carros*/
		// 	if(posicoes[i].ocupado == 0){
		// 		printf("posicao i == %d\n", i+1);
		// 		j = i+1;
		// 		while(j < QNT_CARROS && achou != 1){
		// 				printf("achou == %d\n",posicoes[i].ocupado );
					
		// 			if(posicoes[j].ocupado == 1 ){
		// 				posicoes[j].ocupado = 0;
		// 				posicoes[i].ocupado = 1;
		// 				aux = posicoes[i].carro_ocupado;
		// 				posicoes[i].carro_ocupado = posicoes[j].carro_ocupado;
		// 				posicoes[j].carro_ocupado = aux;
		// 				printf("achou == %d\n",achou );
		// 				achou = 1;
		// 			}

		// 			j++;
		// 		}
		// 		achou=0;
		// 	}
		// }
			
		printf("\n\n--------------------------Posicao atual --------------------------------\n");
		printf("Tempo : %d\n", tempo_durante);

		for( i=0;i<QNT_CARROS;i++){
			if(posicoes[i].ocupado == 1){
				printf("Carro %d na posicao %d\n", posicoes[i].carro_ocupado, posicoes[i].posicao );
			}
			else{
				printf("Carro %d enchendo tanque\n", posicoes[i].carro_ocupado );

			}
		}
		printf("\n\n");
		tempo_momentaneo = time(NULL);
		if(tempo_durante <=0){
			fim = 1;
			printf("*******************************ACABOU**********************************\n");
			for(i=0;i<QNT_CARROS;i++){
				posicoes[i].ocupado =1;
			}	
			printf("\n\n--------------------------Posicao atual --------------------------------\n");
			printf("Tempo : %d\n", tempo_durante);

			for( i=0;i<QNT_CARROS;i++){
				printf("Carro %d na posicao %d\n", posicoes[i].carro_ocupado, posicoes[i].posicao );
			}
			exit(0); /*Fecha o programa */
		}
		pthread_mutex_unlock(&gasolina_lock);
		pthread_mutex_unlock(&lock);
		pthread_mutex_unlock(&turno);
	}
}

void * frentistas_gasolina(void *arg){
    int id = *((int *) arg);
    while(fim == 0){

    	pthread_mutex_lock(&turno);
    	pthread_mutex_lock(&gasolina_lock);
    	/*colocando frentista para dormir caso nao tenham carros esperando*/
    	if(bombas_gasolina == 0){
    		printf("Frentista %d descansando \n", id );
    		pthread_cond_wait(&frentista_status, &gasolina_lock);
    	}
    	pthread_mutex_unlock(&gasolina_lock);
    	pthread_mutex_unlock(&turno);
    	
    	printf("Frentista %d colocando gasolina no carro\n", id);
    	sleep(3); /*Quantidade de tempo em que mexera com gasolina*/


    	pthread_mutex_lock(&gasolina_lock);
    	bombas_gasolina --;
    	enchendo_gasolina=0;
    	pthread_cond_signal(&carro_status2); /* Liberou um carro*/
    	pthread_mutex_unlock(&gasolina_lock);
    	sleep(2);
    	printf("Frenstista %d liberou um carro\n", id);
    	pthread_mutex_lock(&gasolina_lock);
    	bomba_cheia = 0;
    	pthread_cond_signal(&carro_status);

    	pthread_mutex_unlock(&gasolina_lock);

    }
	

}

void * fireman(void *arg){

	while(fim ==0){
		sleep(30);
		pthread_mutex_lock(&turno);
		pthread_mutex_lock(&gasolina_lock);
		pthread_mutex_lock(&lock);
		bombeiro_car =1;
		printf("\n\n----------------------------------- WARNING -------------------------------\n\n");
	    printf("Carros bateram, sera necessario a entrada dos bombeiros\n");
		printf("Bombeiro salvando quem puder\n");
		sleep(rand()%10 + 10);
		pthread_mutex_unlock(&lock);
		pthread_mutex_unlock(&gasolina_lock);
		bombeiro_car = 0;
		pthread_mutex_unlock(&turno);
	}
}

void * pista_corrida(void *arg){
	int posicao_atual;
    int id = *((int *) arg);
    int i, j, aux;

    while(fim==0){
    	pthread_mutex_lock(&turno);
    	pthread_mutex_lock(&lock);
    	printf("\n");
    	/*-----------------------------------------------funcao para ocupar posicao ---------------*/
    	// posicao_atual = ocupa_posicao((void*)arg);
    	i=0;
    	int achou = 0;
	    while(achou ==0){
			for( j=0;j<QNT_CARROS;j++){
				if(posicoes[j].carro_ocupado == id){
					break;
				}
			}
			if(posicoes[i].ocupado == 0 ){
				if(posicoes[i].carro_ocupado != id){ /* verificaçao se mudou de  posição (houve ultrapassagem) o carro muda tb com o que estava no lugar*/
					aux = posicoes[i].carro_ocupado;
					posicoes[i].carro_ocupado = id;
					posicoes[j].carro_ocupado = aux;	
				}
				else{
					posicoes[i].carro_ocupado = id;
				}
				posicoes[i].ocupado = 1;
	    		printf("Carro %d ocupou %d posicao\n",posicoes[i].carro_ocupado, 1+i );
				achou = 1;
				posicao_atual = i;
			}
			i++;
			if(i==QNT_CARROS){
				i=0;
			}
		}
    	/*-----------------------------------------------FIM funcao para ocupar posicao ---------------*/
    	pthread_mutex_unlock(&lock);
    	pthread_mutex_unlock(&turno);
    	
        sleep(rand()%10 + 4); /*andando com a gasolina aleatoria que ele tem*/
    	if(bombeiro_car == 0){ /*Ao bombeiro entrar em campo, os carros "encostam e voltam direto para a pista quando ele sair"*/ 

    		printf("Carro %d esta indo abastecer\n", id );
	    	pthread_mutex_lock(&gasolina_lock);	
	    	posicoes[posicao_atual].ocupado=0; /*O carro deixou de ocupar essa posicao por ter parado*/
	    	bombas_gasolina++;
	    	if(bombas_gasolina ==1){
	    		printf("Acordando frentista\n");	
	    		pthread_cond_signal(&frentista_status);
	    	}
	    	if(bombas_gasolina > QNT_BOMBAS_GASOLINA){
	    		bomba_cheia = 1;
	    	}
	    	while(bomba_cheia == 1){
	    		printf("Carro %d chegou e está esperando na fila\n", id );
	    		pthread_cond_wait(&carro_status, &gasolina_lock);
	    	}
	    	enchendo_gasolina = 1;
	    	while(enchendo_gasolina == 1){
	    		printf("Carro %d esperando o frentista liberar\n", id );
	    		pthread_cond_wait(&carro_status2, &gasolina_lock); 
	    	}

	    	pthread_mutex_unlock(&gasolina_lock);

	    	printf("Carro %d voltando para a pista\n", id );
    	}
    	else{
    		printf("Carro %d encostou para entrar na pista quando o bombeiro sair\n", id );
    		sleep(10);
    	}

    }

}


int main(){
	pthread_t carros[QNT_CARROS];
	pthread_t frentistas[QNT_FRENTISTAS];
	pthread_t printar_tela;
	pthread_t bombeiro;
	int i, *cont;
	tempo_inicio = time(NULL);
	 
    /* Inicializar bombas de gasolina*/
    bombas_gasolina = 0;
	/*INicializar as posições que serão pegas pelos carros*/
	for(i=0;i<QNT_CARROS;i++){
		posicoes[i].posicao = i+1;
		posicoes[i].ocupado = 0;
	}

	/*Inicializar as threads dos carros*/
	for(i=0;i<QNT_CARROS;i++){
        cont = (int*) malloc(sizeof(int));
        *cont = i;
        pthread_create(&carros[i],NULL,&pista_corrida,(void*)cont);
    }	

    for(i=0;i<QNT_FRENTISTAS;i++){
        cont = (int*) malloc(sizeof(int));
        *cont = i;
        pthread_create(&frentistas[i],NULL,&frentistas_gasolina,(void*)cont);
    }

    cont = 0;
    /*Thread criada apenas para entrar no loop de print na tela*/
    pthread_create(&printar_tela,NULL,&print_posicoes,(void*)cont);
    
    /*Thread criada apenas para colocar o bombeiro para rodar*/
    pthread_create(&bombeiro,NULL,&fireman,(void*)cont);

    for(i = 0; i < QNT_CARROS; i++){
        if(pthread_join(carros[i], NULL))
        {
            printf("Could not join thread %d\n", i);
            return -1;
        }
    }
    for(i = 0; i < QNT_CARROS; i++){
        if(pthread_join(frentistas[i], NULL))
        {
            printf("Could not join thread %d\n", i);
            return -1;
        }
    }
   
    pthread_join(printar_tela, NULL);
    pthread_join(bombeiro, NULL);

	return 0;
}