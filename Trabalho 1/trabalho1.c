/* 
    Universidade Federal do Rio de Janeiro
    EEL770 - Sistemas  Operacionais
    Trabalho 1 - Gabriel de Lima Moura (118045099)
*/

/* obs.: O trabalho foi desenvolvido em um sistema linux (ubuntu) */

/* Importacoes necessarias */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

/* Definicao do tipo bool e das variaveis que comandam a execucao das tarefas */
typedef enum { false, true } bool;
bool executarTarefa1 = false; 
bool executarTarefa2 = false;
bool executarFinalizacao = false;


/* Definicao do signal_handler para controlar o recebimento dos sinais */
void signal_handler (int sigNumber){
    if (sigNumber == SIGUSR1){
        printf("\n-- SIGUSR1 recebido --\n");
        executarTarefa1 = true;
    }
    else if (sigNumber == SIGUSR2){
        printf("\n-- SIGUSR2 recebido --\n");
        executarTarefa2 = true;
    }
    else if (sigNumber == SIGTERM){
        printf("\n-- SIGTERM recebido --\n");
        executarFinalizacao = true;
    }
}

/* Funcao Principal*/
int main(void) {
    
    /* Inicializacao das variaveis*/
    pid_t childpid;
    int descritoresPipe[2];
    int comandoParaExecutar = 0;

    printf ("\n Iniciando o processo pai, pid: %d\n", getpid());
    
    if (signal (SIGUSR1, signal_handler) == SIG_ERR){
        printf ("------ Erro ao tratar o sinal SIGUSR1 ------\n");
    }
    if (signal (SIGUSR2, signal_handler) == SIG_ERR){
        printf ("------ Erro ao tratar o sinal SIGUSR2 ------\n");
    }
    if (signal (SIGTERM, signal_handler) == SIG_ERR){
        printf ("------ Erro ao tratar o sinal SIGTERM ------\n");
    }
    
    /* Loop Infinito esperando receber os sinais e os comandos para execucao das tarefas*/
    while(1){

        ////////////////////////////////////////////////// TAREFA 1
        if (executarTarefa1){

            /* ---- 1. O pai cria um pipe.*/
            if (pipe (descritoresPipe)) {
            printf ("------ Erro na criacao do Pipe ------\n");
            exit(1);
            }

            /* ---- 2. O pai cria um processo filho.*/
            childpid = fork(); 
            if (childpid < 0) {
                printf("------ Erro na execucao do fork ------\n");
                exit(1);
            }

            else if (childpid > 0) {
            /* ---- 3. O pai fica esperando a finalização do filho.*/
                wait(NULL);

            /* ---- 7. O pai lê o valor recebido e guarda a resposta do filho na variável denominada comandoParaExecutar.*/
                read(descritoresPipe[0], &comandoParaExecutar, sizeof(comandoParaExecutar));
            
            /* ---- 8. O pai fecha qualquer ponta aberta do pipe.*/
                close (descritoresPipe[0]); // Pai fecha a ponta de leitura do pipe
                close (descritoresPipe[1]); // Pai fecha a ponta de escrita do pipe
            }

            else {
           
            /* ---- 4. O filho sorteia um número inteiro aleatório de 1 a 100.*/    
                time_t t;
                srand((double) time(&t));
                int num_aleatorio = rand() % 101;
            
            /* ---- 5. O filho imprime o número na tela e envia esse número para o pai.*/
                printf("O número sorteado foi : %d\n",num_aleatorio);
                write (descritoresPipe[1],&num_aleatorio,sizeof(num_aleatorio)); 
            
            /* ---- 6. O filho fecha qualquer ponta aberta do pipe e finaliza.*/
                close (descritoresPipe[0]); // Filho fecha a ponta de leitura do pipe
                close (descritoresPipe[1]); // Filho fecha a ponta de escrita do pipe
                return 0;
            }   
            executarTarefa1 = false;
        }

        ////////////////////////////////////////////////// TAREFA 2
        if (executarTarefa2){

            /* ---- 1 e 2. O pai cria um processo filho e envia ao filho o conteúdo da variável comandoParaExecutar por heranca.*/
            childpid = fork(); 
            if (childpid < 0) {
                printf("------ Erro na execucao do fork ------\n");
                exit(1);
            }  

            else if (childpid > 0) {
            /* ---- 3. O pai fica esperando a finalização do filho.*/
                wait(NULL);
            }

            else {

                // Se o numero for zero o filho imprime “Nao ha comando a executar” e finaliza.
                if (comandoParaExecutar == 0){
                    printf("Nao ha comando a executar\n\n");
                }

                // Se o numero for diferente de zero e par, o filho executa o comando “ping 8.8.8.8 -c 5”.
                else if (comandoParaExecutar % 2 == 0){
                    execlp("/bin/ping","ping","8.8.8.8","-c","5",NULL);
                }

                // Se o numero for impar, o filho executa o comando “ping paris.testdebit.info -c 5 -i 2”.
                else{
                    execlp("/bin/ping","ping","paris.testdebit.info","-c","5","-i","2",NULL);
                }
                return 0;
            }
            executarTarefa2 = false;
        }

        ////////////////////////////////////////////////// Finalizacao do programa
        if (executarFinalizacao){
            printf("Finalizando o disparador...\n\n");
            return 0;
        }

        sleep(1);
    }

    return 0;
}