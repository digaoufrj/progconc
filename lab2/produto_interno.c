#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Variáveis globais
long int N;
int T;
float *vetor_a;
float *vetor_b;

// Função executada pelas threads para calcular o produto interno
void *calcula_produto_interno(void *arg) {
    long int id = (long int) arg;
    long int inicio = id * (N / T);
    long int fim = (id == T-1) ? N : (id + 1) * (N / T);
    double *produto_local = (double*) malloc(sizeof(double));
    *produto_local = 0.0;

    for(long int i = inicio; i < fim; i++) {
        *produto_local += vetor_a[i] * vetor_b[i];
    }

    pthread_exit((void*) produto_local);
}

int main(int argc, char*argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Uso: %s <arquivo de entrada> <numero de threads> <arquivo de saida>\n", argv[0]);
        return 1;
    }

    FILE *arq_entrada = fopen(argv[1], "rb");
    if(!arq_entrada) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada\n");
        return 2;
    }

    fread(&N, sizeof(long int), 1, arq_entrada);
    vetor_a = (float*) malloc(sizeof(float) * N);
    vetor_b = (float*) malloc(sizeof(float) * N);
    fread(vetor_a, sizeof(float), N, arq_entrada);
    fread(vetor_b, sizeof(float), N, arq_entrada);
    double produto_interno_esperado;
    fread(&produto_interno_esperado, sizeof(double), 1, arq_entrada);
    fclose(arq_entrada);

    T = atoi(argv[2]);
    if(T > N) T = N;

    pthread_t *threads = (pthread_t*) malloc(T * sizeof(pthread_t));
    double produto_interno_concorrente = 0.0;

    for(long int i = 0; i < T; i++) {
        pthread_create(&threads[i], NULL, calcula_produto_interno, (void*) i);
    }

    for(int i = 0; i < T; i++) {
        double *resultado;
        pthread_join(threads[i], (void**) &resultado);
        produto_interno_concorrente += *resultado;
        free(resultado);
    }

    FILE *arq_saida = fopen(argv[3], "w");
    if(!arq_saida) {
        fprintf(stderr, "Erro ao abrir o arquivo de saida\n");
        return 3;
    }
    
    fprintf(arq_saida, "Produto interno sequencial esperado: %.6f\n", produto_interno_esperado);
    fprintf(arq_saida, "Produto interno concorrente calculado: %.6f\n", produto_interno_concorrente);

    double variacao_relativa = (produto_interno_esperado - produto_interno_concorrente) / produto_interno_esperado;
    fprintf(arq_saida, "Variacao relativa: %.6f\n", variacao_relativa);

    fclose(arq_saida);
    free(vetor_a);
    free(vetor_b);
    free(threads);

    return 0;
}

