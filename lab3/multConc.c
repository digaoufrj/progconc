#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

float *matriz1, *matriz2, *resultado;
int linhas1, colunas1, linhas2, colunas2, nthreads;

typedef struct {
    int id;
} tArgs;

void* multiplicaConcorrente(void* arg) {
    int id = ((tArgs*)arg)->id;
    for (int i = id; i < linhas1; i += nthreads)
        for (int j = 0; j < colunas2; j++) {
            resultado[i * colunas2 + j] = 0;
            for (int k = 0; k < colunas1; k++)
                resultado[i * colunas2 + j] += matriz1[i * colunas1 + k] * matriz2[k * colunas2 + j];
        }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Formato: %s <arquivo_matriz1> <arquivo_matriz2> <arquivo_saida> <qtd_threads>\n", argv[0]);
        return 1;
    }

    FILE *arq_1 = fopen(argv[1], "rb"), *arq_2 = fopen(argv[2], "rb"), *arquivo_saida = fopen(argv[3], "wb");
    nthreads = atoi(argv[4]);

    if (!arq_1 || !arq_2 || !arquivo_saida || nthreads <= 0) {
        fprintf(stderr, "Erro ao abrir arquivo ou numero de threads invalido\n");
        return 2;
    }

    if (fread(&linhas1, sizeof(int), 1, arq_1) != 1 || fread(&colunas1, sizeof(int), 1, arq_1) != 1 ||
        fread(&linhas2, sizeof(int), 1, arq_2) != 1 || fread(&colunas2, sizeof(int), 1, arq_2) != 1) {
        fprintf(stderr, "Erro ao ler dimensões das matrizes\n");
        return 4;
    }

    matriz1 = (float*)malloc(linhas1 * colunas1 * sizeof(float));
    matriz2 = (float*)malloc(linhas2 * colunas2 * sizeof(float));
    if (!matriz1 || !matriz2) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 3;
    }
    if (fread(matriz1, sizeof(float), linhas1 * colunas1, arq_1) != linhas1 * colunas1 ||
        fread(matriz2, sizeof(float), linhas2 * colunas2, arq_2) != linhas2 * colunas2) {
        fprintf(stderr, "Erro ao ler dados das matrizes\n");
        return 4;
    }

    fclose(arq_1);
    fclose(arq_2);

    if (colunas1 != linhas2) {
        fprintf(stderr, "Dimensões não podem ser multiplicadas\n");
        return 5;
    }

    resultado = (float*)malloc(linhas1 * colunas2 * sizeof(float));
    pthread_t *threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    tArgs *args = (tArgs*)malloc(nthreads * sizeof(tArgs));

    if (!resultado || !threads || !args) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 6;
    }

    double inicio, final;
    GET_TIME(inicio);

    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        if (pthread_create(&threads[i], NULL, multiplicaConcorrente, (void*)&args[i])) {
            fprintf(stderr, "Erro ao criar thread\n");
            return 7;
        }
    }

    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    GET_TIME(final);
    printf("Multiplicação concorrente com %d threads concluida em %lf segundos\n", nthreads, final - inicio);

    fwrite(&linhas1, sizeof(int), 1, arquivo_saida);
    fwrite(&colunas2, sizeof(int), 1, arquivo_saida);
    fwrite(resultado, sizeof(float), linhas1 * colunas2, arquivo_saida);

    fclose(arquivo_saida);
    free(matriz1);
    free(matriz2);
    free(resultado);
    free(threads);
    free(args);

    return 0;
}

