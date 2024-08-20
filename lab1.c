#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int *vetor;
    int pInicial, pFinal;
} t_Args;

void *soma(void *arg) {
    t_Args *args = (t_Args *) arg;

    for (int i = args->pInicial; i < args->pFinal; i++)
        args->vetor[i] += 1;

    free(arg);
    pthread_exit(NULL);
}

void inicializaVetor(int *vetor, int N) {
    printf("Inicializando vetor...\n");
    for (int i = 0; i < N; i++)
        vetor[i] = i;
    printf("Vetor inicializado.\n");
}

void verificaVetor(int *vetor, int N) {
    printf("Verificando o vetor...\n");
    for (int i = 0; i < N; i++) {
        if (vetor[i] != i + 1) {
            printf("Erro na posição %d: correto %d, calculado %d\n", i, i + 1, vetor[i]);
            return;
        }
    }
    printf("Vetor verificado corretamente, valores corretos.\n");
}

void criaThreads(pthread_t *threadId, int M, int *vetor, int N) {
    int threadItems = N / M, resto = N % M;

    for (int i = 0; i < M; i++) {
        t_Args *args = (t_Args *) malloc(sizeof(t_Args));
        if (args == NULL) {
            printf("Erro ao alocar memória para args\n");
            exit(2);
        }

        args->vetor = vetor;
        args->pInicial = i * threadItems + (i < resto ? i : resto);
        args->pFinal = args->pInicial + threadItems + (i < resto ? 1 : 0);

        if (pthread_create(&threadId[i], NULL, soma, (void *) args)) {
            printf("Erro ao criar thread %d\n", i);
            exit(3);
        }
    }
    printf("Todas as threads foram criadas.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <N> <M>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]), M = atoi(argv[2]);
    int *vetor = (int *) malloc(N * sizeof(int));
    if (vetor == NULL) {
        printf("Erro ao alocar memória para o vetor\n");
        return 2;
    }

    pthread_t threadId[M];

    inicializaVetor(vetor, N);

    criaThreads(threadId, M, vetor, N);

    for (int i = 0; i < M; i++) {
        if (pthread_join(threadId[i], NULL)) {
            printf("Erro ao esperar thread %d\n", i);
            return 3;
        }
    }
    verificaVetor(vetor, N);

    free(vetor);
    printf("Memória liberada e programa finalizado.\n");
    return 0;
}
