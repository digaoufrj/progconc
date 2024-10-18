#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define N 10 // Tamanho do buffer e quantos caracteres vamos ler de uma vez

// Buffers e seus tamanhos
char buffer1[N + 1]; // +1 para ter espaço pro '\0' (final da string)
char buffer2[N + 2]; // Espaço extra pro '\n' e '\0'

// Semáforos pra controlar a sincronia
sem_t sem_fill_buffer1;
sem_t sem_fill_buffer2;

// Mutexes pra evitar problemas de concorrência nos buffers
pthread_mutex_t mutex_buffer1;
pthread_mutex_t mutex_buffer2;

int eof_reached = 0; // Variável que indica se chegamos no fim do arquivo
int bytesRead = 0;   // Quantidade de bytes que lemos

// Função da thread 1: Lê o arquivo e coloca os dados no buffer1
void *thread1(void *arg)
{
    FILE *file = fopen("input.txt", "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Lê enquanto ainda houver dados no arquivo
    while ((bytesRead = fread(buffer1, sizeof(char), N, file)) > 0)
    {
        // Adiciona o caractere nulo no final da string
        buffer1[bytesRead] = '\0';

        // Diz que o buffer1 está cheio
        sem_post(&sem_fill_buffer1);

        // Espera a thread 2 processar o buffer1 antes de continuar
        sem_wait(&sem_fill_buffer2);
    }

    // Marca que acabamos de ler tudo do arquivo
    pthread_mutex_lock(&mutex_buffer1);
    eof_reached = 1;
    sem_post(&sem_fill_buffer1); // Avisa a thread 2 que é a última vez
    pthread_mutex_unlock(&mutex_buffer1);

    fclose(file);
    pthread_exit(NULL);
}

// Função da thread 2: Processa o buffer1 e adiciona '\n' no buffer2
void *thread2(void *arg)
{
    while (1)
    {
        // Espera até que o buffer1 esteja cheio
        sem_wait(&sem_fill_buffer1);

        // Bloqueia o acesso aos buffers
        pthread_mutex_lock(&mutex_buffer1);
        pthread_mutex_lock(&mutex_buffer2);

        // Verifica se chegamos no fim do arquivo e se o buffer está vazio
        if (eof_reached && bytesRead == 0)
        {
            pthread_mutex_unlock(&mutex_buffer1);
            pthread_mutex_unlock(&mutex_buffer2);
            sem_post(&sem_fill_buffer2); // Libera a thread3
            break;
        }

        // Copia os dados do buffer1 pro buffer2
        strcpy(buffer2, buffer1);

        // Adiciona o '\n' no final do bloco
        if (bytesRead == N)
        {
            buffer2[N] = '\n';
            buffer2[N + 1] = '\0'; // Garante que buffer2 termina com '\0'
        }
        else
        {
            buffer2[bytesRead] = '\n';     // Adiciona '\n' no final do bloco parcial
            buffer2[bytesRead + 1] = '\0'; // Garante que buffer2 termina com '\0'
        }

        // Sinaliza que o buffer2 está pronto pra ser impresso
        sem_post(&sem_fill_buffer2);

        // Desbloqueia os buffers
        pthread_mutex_unlock(&mutex_buffer1);
        pthread_mutex_unlock(&mutex_buffer2);
    }
    pthread_exit(NULL);
}

// Função da thread 3: Imprime o conteúdo do buffer2 na tela
void *thread3(void *arg)
{
    while (1)
    {
        // Espera o buffer2 ficar cheio
        sem_wait(&sem_fill_buffer2);

        // Bloqueia o acesso ao buffer2
        pthread_mutex_lock(&mutex_buffer2);

        // Verifica se o fim do arquivo foi alcançado e não há mais dados
        if (eof_reached && bytesRead == 0)
        {
            pthread_mutex_unlock(&mutex_buffer2);
            break;
        }

        // Imprime o que está no buffer2
        printf("%s", buffer2);

        // Desbloqueia o buffer2
        pthread_mutex_unlock(&mutex_buffer2);

        // Sinaliza pra thread1 que ela pode continuar a leitura
        sem_post(&sem_fill_buffer2);
    }
    pthread_exit(NULL);
}

int main()
{
    // Inicializa os semáforos
    sem_init(&sem_fill_buffer1, 0, 0);
    sem_init(&sem_fill_buffer2, 0, 0);

    // Inicializa os mutexes
    pthread_mutex_init(&mutex_buffer1, NULL);
    pthread_mutex_init(&mutex_buffer2, NULL);

    // Cria as três threads
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_create(&t3, NULL, thread3, NULL);

    // Espera as threads terminarem
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    // Destrói os semáforos e mutexes
    sem_destroy(&sem_fill_buffer1);
    sem_destroy(&sem_fill_buffer2);
    pthread_mutex_destroy(&mutex_buffer1);
    pthread_mutex_destroy(&mutex_buffer2);

    return 0;
}
