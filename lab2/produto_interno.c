#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long int N;
int T;
float *vet1;
float *vet2;

void *calcula_pi(void *arg) {
    long int id = (long int) arg;
    long int inicio = id * (N / T);
    long int fim = (id == T-1) ? N : (id + 1) * (N / T);
    double *produto = (double*) malloc(sizeof(double));
    *produto = 0.0;

    for(long int i = inicio; i < fim; i++) {
        *produto += vet1[i] * vet2[i];
    }

    pthread_exit((void*) produto);
}

int main(int argc, char*argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Digite: %s (arquivo de entrada) (qtd threads) (arquivo de saida)\n", argv[0]);
        return 1;
    }

    FILE *arq_entrada = fopen(argv[1], "rb");
    if(!arq_entrada) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada\n");
        return 2;
    }

    fread(&N, sizeof(long int), 1, arq_entrada);
    vet1 = (float*) malloc(sizeof(float) * N);
    vet2 = (float*) malloc(sizeof(float) * N);
    fread(vet1, sizeof(float), N, arq_entrada);
    fread(vet2, sizeof(float), N, arq_entrada);
    double pi_sequencial;
    fread(&pi_sequencial, sizeof(double), 1, arq_entrada);
    fclose(arq_entrada);

    T = atoi(argv[2]);
    if(T > N) T = N;

    pthread_t *threads = (pthread_t*) malloc(T * sizeof(pthread_t));
    double pi_concorrente = 0.0;

    for(long int i = 0; i < T; i++) {
        pthread_create(&threads[i], NULL, calcula_pi, (void*) i);
    }

    for(int i = 0; i < T; i++) {
        double *resultado;
        pthread_join(threads[i], (void**) &resultado);
        pi_concorrente += *resultado;
        free(resultado);
    }

    FILE *arq_saida = fopen(argv[3], "w");
    if(!arq_saida) {
        fprintf(stderr, "Erro ao abrir o arquivo de saida\n");
        return 3;
    }
    
    fprintf(arq_saida, "Produto interno código sequencial: %.6f\n", pi_sequencial);
    fprintf(arq_saida, "Produto interno código concorrente: %.6f\n", pi_concorrente);

    double variacao = (pi_sequencial - pi_concorrente) / pi_sequencial;
    fprintf(arq_saida, "Variacao relativa: %.6f\n", variacao);

    fclose(arq_saida);
    free(vet1);
    free(vet2);
    free(threads);

    return 0;
}
