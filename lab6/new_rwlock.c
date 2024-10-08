// Programa concorrente que cria e faz operacoes sobre uma lista de inteiros

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include "list_int.h" 

#define QTDE_OPS 5000 // quantidade de operações sobre a lista (inserção, remoção, consulta) - diminui pra 5000 pra facilitar a visualização do log
#define QTDE_INI 100  // quantidade de inserções iniciais na lista
#define MAX_VALUE 100 // valor máximo a ser inserido

// -------------------------------parte fundamental lab 6 --------------------------------------------

// Estrutura rwlock dando prioridade a escrita
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_read;
    pthread_cond_t cond_write;
    int leitores;       // Conta os as operações de leitura ativos
    int escrita_espera; // Conta  as operações de escrita esperando
    int escrita_ativa;  // Mostra se um escritor está ativo
} rwlock_t;

// Inicializa o rwlock
void rwlock_init(rwlock_t *lock)
{
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->cond_read, NULL);
    pthread_cond_init(&lock->cond_write, NULL);
    lock->leitores = 0;
    lock->escrita_espera = 0;
    lock->escrita_ativa = 0;
}

// Lock de leitura
void rwlock_rdlock(rwlock_t *lock)
{
    pthread_mutex_lock(&lock->mutex);
    while (lock->escrita_ativa) // Bloqueia se houver um escritor ativo
    {
        printf("Thread %ld: bloqueada para ler devido à prioridade de escrita.\n", pthread_self());
        pthread_cond_wait(&lock->cond_read, &lock->mutex);
    }
    lock->leitores++;
    pthread_mutex_unlock(&lock->mutex);
}

// Unlock de leitura
void rwlock_rdunlock(rwlock_t *lock)
{
    pthread_mutex_lock(&lock->mutex);
    lock->leitores--;
    if (lock->leitores == 0 && lock->escrita_espera > 0)
    {
        pthread_cond_signal(&lock->cond_write); // Notifica o escritor se houver escritores esperando
    }
    pthread_mutex_unlock(&lock->mutex);
}

// Lock de escrita
void rwlock_wrlock(rwlock_t *lock)
{
    pthread_mutex_lock(&lock->mutex);
    lock->escrita_espera++;
    while (lock->leitores > 0 || lock->escrita_ativa) // Bloqueia se houver leitores ativos ou um escritor ativo
    {
        pthread_cond_wait(&lock->cond_write, &lock->mutex);
    }
    lock->escrita_espera--;
    lock->escrita_ativa = 1;
    pthread_mutex_unlock(&lock->mutex);
}

// Unlock de escrita
void rwlock_wrunlock(rwlock_t *lock)
{
    pthread_mutex_lock(&lock->mutex);
    lock->escrita_ativa = 0;
    pthread_cond_broadcast(&lock->cond_read); // Notifica leitores
    pthread_cond_signal(&lock->cond_write);   // Notifica escritores esperando
    pthread_mutex_unlock(&lock->mutex);
}

// Destroi o rwlock
void rwlock_destroy(rwlock_t *lock)
{
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->cond_read);
    pthread_cond_destroy(&lock->cond_write);
}

// -------------------------------parte fundamental lab 6 --------------------------------------------

// Lista compartilhada iniciada
struct list_node_s *head_p = NULL;
// Quantidade de threads no programa
int nthreads;

// rwlock de exclusão mútua
rwlock_t rwlock;

// Tarefa das threads
void *tarefa(void *arg)
{
    long int id = (long int)arg;
    int op;
    int in = 0, out = 0, read = 0;

    // Realiza operações de consulta (98%), inserção (1%) e remoção (1%)
    for (long int i = id; i < QTDE_OPS; i += nthreads)
    {
        op = rand() % 100;
        if (op < 98)
        {
            rwlock_rdlock(&rwlock);        /* Lock de LEITURA */
            Member(i % MAX_VALUE, head_p); /* Ignore return value */
            rwlock_rdunlock(&rwlock);
            read++;
        }
        else if (98 <= op && op < 99)
        {
            rwlock_wrlock(&rwlock);         /* Lock de ESCRITA */
            Insert(i % MAX_VALUE, &head_p); /* Ignore return value */
            rwlock_wrunlock(&rwlock);
            in++;
        }
        else
        {
            rwlock_wrlock(&rwlock);         /* Lock de ESCRITA */
            Delete(i % MAX_VALUE, &head_p); /* Ignore return value */
            rwlock_wrunlock(&rwlock);
            out++;
        }
    }
    // Registra a quantidade de operações realizadas por tipo
    printf("Thread %ld: in=%d out=%d read=%d\n", id, in, out, read);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t *tid;
    double ini, fim, delta;

    // Verifica se o número de threads foi passado na linha de comando
    if (argc < 2)
    {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    // Insere os primeiros elementos na lista
    for (int i = 0; i < QTDE_INI; i++)
        Insert(i % MAX_VALUE, &head_p); /* Ignore return value */

    // Aloca espaço de memória para o vetor de identificadores de threads no sistema
    tid = malloc(sizeof(pthread_t) * nthreads);
    if (tid == NULL)
    {
        printf("--ERRO: malloc()\n");
        return 2;
    }

    // Tomada de tempo inicial
    GET_TIME(ini);

    // Inicializa a variável rwlock
    rwlock_init(&rwlock);

    // Cria as threads
    for (long int i = 0; i < nthreads; i++)
    {
        if (pthread_create(tid + i, NULL, tarefa, (void *)i))
        {
            printf("--ERRO: pthread_create()\n");
            return 3;
        }
    }

    // Aguarda as threads terminarem
    for (int i = 0; i < nthreads; i++)
    {
        if (pthread_join(*(tid + i), NULL))
        {
            printf("--ERRO: pthread_join()\n");
            return 4;
        }
    }

    // Tomada de tempo final
    GET_TIME(fim);
    delta = fim - ini;
    printf("Tempo: %lf\n", delta);

    // Libera o rwlock
    rwlock_destroy(&rwlock);
    // Libera o espaço de memória do vetor de threads
    free(tid);
    // Libera o espaço de memória da lista
    Free_list(&head_p);

    return 0;
}
