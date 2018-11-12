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

#define QNT_CARROS 6
#define QNT_BOMBAS_GASOLINA 1
#define QNT_FRENTISTAS 1


typedef struct{
	int posicao;
	int ocupado;
	int carro_ocupado;

}posicoes_carros;

posicoes_carros posicoes[QNT_CARROS];

pthread_cond_t carro_status = PTHREAD_COND_INITIALIZER;
pthread_cond_t carro_status2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t frentista_status = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gasolina_lock = PTHREAD_MUTEX_INITIALIZER;
int carros_pista, fim = 0, enchendo_gasolina=0; /* Ao chegar em um, significa que acabará a corrida*/
int bombas_gasolina = QNT_BOMBAS_GASOLINA;

int vet[100];

int ocupa_posicao(void *arg){
	int i=0, j, aux;
    int id = *((int *) arg);
	while(1){
		for( j=0;j<QNT_CARROS;j++){
			if(posicoes[j].carro_ocupado == j){
				break;
			}
		}
		if(posicoes[i].ocupado == 0 ){
			if(posicoes[i].carro_ocupado != id){
				aux = posicoes[i].carro_ocupado;
				posicoes[i].carro_ocupado = id;
				posicoes[j].carro_ocupado = aux;	
			}
			else{
				posicoes[i].carro_ocupado = id;
			}
			posicoes[i].ocupado = 1;
    		printf("id = %d ocupou %d\n",posicoes[i].carro_ocupado, 1+i );
			return i;
		}
		i++;
		if(i==QNT_CARROS){
			i=0;
		}
	}

	return 0;
}

void * print_posicoes(void *arg){
	while(1){
		sleep(4);
		printf("--------------------------Posicao atual --------------------------------\n");
		for(int i=0;i<QNT_CARROS;i++){
			if(posicoes[i].ocupado == 1){
				printf("Carro %d na posicao %d\n", posicoes[i].carro_ocupado, posicoes[i].posicao );
			}
			else{
				printf("Carro %d enchendo tanque\n", posicoes[i].carro_ocupado );

			}
		}
		printf("\n");
	}
}

void * frentistas_gasolina(void *arg){
    int id = *((int *) arg);
    while(1){

    	pthread_mutex_lock(&gasolina_lock);
    	/*colocando frentista para dormir caso nao tenham carros esperando*/
    	if(bombas_gasolina == 0){
    		printf("Frentista %d descansando \n", id );
    		pthread_cond_wait(&frentista_status, &gasolina_lock);
    	}
    	pthread_mutex_unlock(&gasolina_lock);
    	
    	printf("Frentista colocando gasolina no carro\n");
    	sleep(1); /*Quantidade de tempo em que mexera com gasolina*/

    	pthread_mutex_lock(&gasolina_lock);
    	bombas_gasolina --;
    	enchendo_gasolina=0;
    	pthread_cond_signal(&carro_status2);
    	printf("Frenstista encheu um tanque\n");
    	pthread_cond_signal(&carro_status);

    	pthread_mutex_unlock(&gasolina_lock);

    }
	

}

void * pista_corrida(void *arg){
	int posicao_atual;
    int id = *((int *) arg);
    while(fim==0){
    	pthread_mutex_lock(&lock);
    	printf("\n");
    	/*-----------------------------------------------funcao para ocupar posicao ---------------*/
    	posicao_atual = ocupa_posicao((void*)arg);
    	/*-----------------------------------------------FIM funcao para ocupar posicao ---------------*/
    	pthread_mutex_unlock(&lock);
    	
        sleep(rand()%15 + 4); /*andando com a gasolina aleatoria que ele tem*/
    	printf("Carro %d acabou a gasolina e esta indo abastecer\n", id );
    	pthread_mutex_lock(&gasolina_lock);	
    	posicoes[posicao_atual].ocupado=0; /*O carro deixou de ocupar essa posicao por ter parado*/
    	bombas_gasolina++;
    	if(bombas_gasolina ==1){
    		printf("Acordando frentista\n");	
    		pthread_cond_signal(&frentista_status);
    	}
    	while(bombas_gasolina > QNT_BOMBAS_GASOLINA){

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

}


int main(){
	pthread_t carros[QNT_CARROS];
	pthread_t frentistas[QNT_FRENTISTAS];
	pthread_t printar_tela;
	int i, *cont;
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
   


	return 0;
}