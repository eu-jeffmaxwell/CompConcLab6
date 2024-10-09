#include <stdio.h>
#include <stdlib.h>
#include "list_int.h"
#include <pthread.h>
#include "timer.h"

#define QTDE_OPS 10000000 // quantidade de operacoes sobre a lista (insercao, remocao, consulta)
#define QTDE_INI 100      // quantidade de insercoes iniciais na lista
#define MAX_VALUE 100     // valor maximo a ser inserido

// lista compartilhada
struct list_node_s* head_p = NULL;
// qtde de threads no programa
int nthreads;

// rwlock para controle de exclusão mútua
pthread_rwlock_t rwlock;
// contador de escritores esperando para dar prioridade
int writers_waiting = 0;

// Função modificada para dar prioridade de escrita
void rwlock_readlock_with_priority() {
    // Se houver escritores esperando, bloqueia novas leituras
    while (1) {
        pthread_rwlock_rdlock(&rwlock);
        if (writers_waiting == 0) { // Só permite leitura se não houver escritores esperando
            break;
        }
        pthread_rwlock_unlock(&rwlock); // Libera o lock se houver escritores esperando
        sched_yield(); // Cede o processador para outra thread
    }
}

void rwlock_writelock_with_priority() {
    // Incrementa o contador de escritores esperando
    __sync_fetch_and_add(&writers_waiting, 1);
    pthread_rwlock_wrlock(&rwlock); // Bloqueia para escrita
}

void rwlock_writeunlock_with_priority() {
    // Decrementa o contador de escritores esperando
    __sync_fetch_and_sub(&writers_waiting, 1);
    pthread_rwlock_unlock(&rwlock); // Libera o lock
}

// Tarefa das threads
void* tarefa(void* arg) {
    long int id = (long int) arg;
    int op;
    int in = 0, out = 0, read = 0;

    // Realiza operacoes de consulta (98%), insercao (1%) e remocao (1%)
    for (long int i = id; i < QTDE_OPS; i += nthreads) {
        op = rand() % 100;
        if (op < 98) {
            rwlock_readlock_with_priority();
            //printf("Thread %ld: lendo valor %ld\n", id, i % MAX_VALUE);
            Member(i % MAX_VALUE, head_p); // Consulta
            pthread_rwlock_unlock(&rwlock); // Libera o lock de leitura
            read++;
        } else if (op >= 98 && op < 99) {
            rwlock_writelock_with_priority();
            //printf("Thread %ld: escrevendo valor %ld\n", id, i % MAX_VALUE);
            Insert(i % MAX_VALUE, &head_p); // Inserção
            rwlock_writeunlock_with_priority();
            in++;
        } else if (op >= 99) {
            rwlock_writelock_with_priority();
            //printf("Thread %ld: removendo valor %ld\n", id, i % MAX_VALUE);
            Delete(i % MAX_VALUE, &head_p); // Remoção
            rwlock_writeunlock_with_priority();
            out++;
        }
    }

    // Registra a quantidade de operacoes realizadas por tipo
    printf("Thread %ld: in=%d out=%d read=%d\n", id, in, out, read);
    pthread_exit(NULL);
}

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    pthread_t *tid;
    double ini, fim, delta;

    // Verifica se o número de threads foi passado na linha de comando
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    // Insere os primeiros elementos na lista
    for (int i = 0; i < QTDE_INI; i++)
        Insert(i % MAX_VALUE, &head_p);  /* Ignore return value */

    // Aloca espaco de memoria para o vetor de identificadores de threads
    tid = malloc(sizeof(pthread_t) * nthreads);
    if (tid == NULL) {
        printf("--ERRO: malloc()\n");
        return 2;
    }

    // Tomada de tempo inicial
    GET_TIME(ini);

    // Inicializa o rwlock
    pthread_rwlock_init(&rwlock, NULL);

    // Cria as threads
    for (long int i = 0; i < nthreads; i++) {
        if (pthread_create(tid + i, NULL, tarefa, (void*) i)) {
            printf("--ERRO: pthread_create()\n");
            return 3;
        }
    }

    // Aguarda as threads terminarem
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(*(tid + i), NULL)) {
            printf("--ERRO: pthread_join()\n");
            return 4;
        }
    }

    // Tomada de tempo final
    GET_TIME(fim);
    delta = fim - ini;
    printf("Tempo: %lf\n", delta);

    // Libera o rwlock
    pthread_rwlock_destroy(&rwlock);
    // Libera o espaco de memoria do vetor de threads
    free(tid);
    // Libera o espaco de memoria da lista
    Free_list(&head_p);

    return 0;
}
