/* 
    Universidade Federal do Rio de Janeiro
    EEL770 - Sistemas  Operacionais
    Trabalho 2 - Gabriel de Lima Moura (118045099)
*/

/***************************************** Algumas Observações */
/*O trabalho foi desenvolvido em um sistema linux (ubuntu) e o problema escolhido foi o "The FIFO barbershop (Secao 5.3)*/

/* Para a escolha do número de clientes máximo (parada do programa) e do número de cadeiras podem */
/*                                                 ser alteradas as Macros de configuração abaixo */ 

/* O programa funciona gerando um tempo aleatório para o intervalo de atendimento (barbeiro cortando */
/*                         cabelo de algum cliente) e para a chegada de um novo cliente na barbearia */


/***************************************** MACROS DE CONFIGURAÇÃO (ALTERAR SE NECESSÁRIO) */
#define NUM_CADEIRAS 3
#define NUM_CLIENTES 10

/* Importacoes necessarias */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/***************************************** IMPLEMENTAÇÃO SIMPLES DE UMA FILA (FIFO) */
long unsigned int posicao_primeiro_cliente = 0;
long unsigned int posicao_ultimo_cliente = -1;
unsigned int clientes_na_fila = 0;
long unsigned int array[NUM_CLIENTES];

void AdicionarCLienteNaFila(long unsigned int id){
    if (clientes_na_fila < NUM_CADEIRAS){   
        posicao_ultimo_cliente = (posicao_ultimo_cliente + 1);
        array[posicao_ultimo_cliente] = id;
        clientes_na_fila = clientes_na_fila + 1;
    }
}
long unsigned int RemoverClienteDaFila()
{
    if (clientes_na_fila > 0){
        long unsigned int id = array[posicao_primeiro_cliente];
        posicao_primeiro_cliente = (posicao_primeiro_cliente + 1);
        clientes_na_fila = clientes_na_fila - 1;
        return id;
    }
    else {
        return 0;
    }
}


/* Definicao do tipo bool a ser utilizado*/
typedef enum { false, true } bool;


/* Inicialização do mutex e Variáveis de Condição */
pthread_mutex_t acessar_barbeiro = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t acessar_cadeiras = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t barbeiro_dormindo = PTHREAD_COND_INITIALIZER;
pthread_cond_t cliente_pronto = PTHREAD_COND_INITIALIZER;


/* Algumas Variáveis extras */
unsigned int cadeiras_ocupadas = 0;
long unsigned int cliente_sendo_atendido = 0;
bool dormindo = true;
unsigned int cliente_indice;  


/* Função utilizada para mostrar o status atual*/ 
void mostrarStatus(){
    printf("\n----------------- Status ------------------  \n");
    if (dormindo == true){
        printf(" Barbeiro Dormindo \n Uso das cadeiras (%d/%d)\n",cadeiras_ocupadas,NUM_CADEIRAS);     
    } 
    else{
        printf(" Barbeiro atendendo o cliente %lu \n Uso das cadeiras (%d/%d)\n",cliente_sendo_atendido,cadeiras_ocupadas,NUM_CADEIRAS);     
    }
    printf("-------------------------------------------\n\n");
}         


/* Função para a thread do barbeiro*/
void *barbeiro(void *arg){ 

    /* Status Inicial*/
    mostrarStatus();
	
    while((cliente_indice < NUM_CLIENTES) || ((cliente_indice == NUM_CLIENTES) && (cadeiras_ocupadas > 0))){

        /* Bloqueia temporariamente o acesso às cadeiras para verificar/alterar as cadeiras ocupadas */
        pthread_mutex_lock(&acessar_cadeiras);
		while (cadeiras_ocupadas == 0 && clientes_na_fila==0) {
			pthread_cond_wait(&cliente_pronto, &acessar_cadeiras);
		}

        /* Começar o antendimento bloqueando o acesso ao barbeiro */
        pthread_mutex_lock(&acessar_barbeiro);
        dormindo = false;
		cadeiras_ocupadas--;
        cliente_sendo_atendido = RemoverClienteDaFila();
		printf(" **** Barbeiro começou a atender o cliente %lu.\n",cliente_sendo_atendido);
		mostrarStatus();
        pthread_mutex_unlock(&acessar_cadeiras);

        /* Simular um cliente sendo atentido com tempo gerado aleatoriamente */
        time_t t;
        srand((double) time(&t));
        unsigned int tempo_aleatorio = 100000 * (30 + rand()%21);
        usleep(tempo_aleatorio);

        /* Terminar o atendimento liberando o acesso ao barbeiro*/
		printf(" **** Barbeiro terminou o atendimento.\n");
        dormindo = true;
        pthread_mutex_unlock(&acessar_barbeiro);
    }

    /* Finaliza o programa quando não há mais clientes para chegar */
    printf("\n-------------------------------------------\n");
    printf("            FIM DOS ATENDIMENTOS           \n");
    printf("-------------------------------------------\n\n");
    pthread_exit(NULL); 
    return NULL;
}


/* Função para as threads dos clientes */
void *cliente(void *arg) {
    long unsigned int cliente_id = pthread_self();
    printf(" **** Cliente %lu chegou na barbearia.\n", cliente_id);

    /* Tenta entrar na fila */
    pthread_mutex_lock(&acessar_cadeiras);
    if (NUM_CADEIRAS - cadeiras_ocupadas >= 1){   
        cadeiras_ocupadas++;
        AdicionarCLienteNaFila(cliente_id);
        printf(" **** Cliente %lu entrou na fila.\n", cliente_id);
        mostrarStatus();
        pthread_cond_signal(&cliente_pronto);
        pthread_mutex_unlock(&acessar_cadeiras);
        pthread_cond_signal(&barbeiro_dormindo);
    }
    else {
        printf(" **** Cliente %lu deixou a barbearia por falta de cadeiras disponíveis.\n", cliente_id);
    }
    pthread_mutex_unlock(&acessar_cadeiras);
    pthread_exit(NULL);  
    return NULL;
}


/* Função para a thread que gera todos os clientes */
void *gerar_clientes(){       
    for (cliente_indice = 0; cliente_indice < NUM_CLIENTES ; cliente_indice++){

        /* Thread para o cliente */
        pthread_t cliente_thread;
        pthread_create(&cliente_thread, NULL, cliente, NULL);
            
        /* Simular um novo cliente entrando na barbearia com um intervalo de tempo aleatorio */
        time_t t;
        srand((double) time(&t));
        unsigned int tempo_aleatorio = 40000 * (30 + rand()%21);
        usleep(tempo_aleatorio);
    }
    pthread_exit(NULL);  
    return NULL;
}


/* Função Principal*/
int main(){
    
    /* Thread para o barbeiro */
    pthread_t barbeiro_thread;
    pthread_create(&barbeiro_thread, NULL, barbeiro, NULL);
    
    /* Thread para a geração de clientes */
    pthread_t gerar_clientes_thread;
    pthread_create(&gerar_clientes_thread, NULL, gerar_clientes, NULL);  

    /* Esperando as threads terminarem */
    pthread_join(barbeiro_thread, NULL);
    pthread_join(gerar_clientes_thread, NULL);

    return 0;
}