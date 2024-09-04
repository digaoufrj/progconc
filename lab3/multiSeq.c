#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

void multiplicaMatriz(float *matriz1, int linhas1, int colunas1, float *matriz2, int linhas2, int colunas2, float *resultado) {
    for (int i = 0; i < linhas1; i++)
        for (int j = 0; j < colunas2; j++) {
            resultado[i * colunas2 + j] = 0;
            for (int k = 0; k < colunas1; k++)
                resultado[i * colunas2 + j] += matriz1[i * colunas1 + k] * matriz2[k * colunas2 + j];
        }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Formato: %s <arquivo_matriz1> <arquivo_matriz2> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    FILE *arq_1 = fopen(argv[1], "rb"), *arq_2 = fopen(argv[2], "rb"), *arquivo_saida = fopen(argv[3], "wb");
    if (!arq_1 || !arq_2 || !arquivo_saida) {
        fprintf(stderr, "Erro ao abrir arquivos\n");
        return 2;
    }

    int linhas1, colunas1, linhas2, colunas2;
    if (fread(&linhas1, sizeof(int), 1, arq_1) != 1 || fread(&colunas1, sizeof(int), 1, arq_1) != 1 ||
        fread(&linhas2, sizeof(int), 1, arq_2) != 1 || fread(&colunas2, sizeof(int), 1, arq_2) != 1) {
        fprintf(stderr, "Erro ao ler dimensões das matrizes\n");
        return 3;
    }

    float *matriz1 = (float *)malloc(linhas1 * colunas1 * sizeof(float)), *matriz2 = (float *)malloc(linhas2 * colunas2 * sizeof(float));
    if (!matriz1 || !matriz2) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 4;
    }
    if (fread(matriz1, sizeof(float), linhas1 * colunas1, arq_1) != (size_t)(linhas1 * colunas1) ||
        fread(matriz2, sizeof(float), linhas2 * colunas2, arq_2) != (size_t)(linhas2 * colunas2)) {
        fprintf(stderr, "Erro ao ler dados das matrizes\n");
        return 5;
    }

    fclose(arq_1);
    fclose(arq_2);

    if (colunas1 != linhas2) {
        fprintf(stderr, "Dimensões não podem ser multiplicadas\n");
        return 6;
    }

    float *resultado = (float *)malloc(linhas1 * colunas2 * sizeof(float));
    if (!resultado) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 7;
    }

    double inicio, final;
    GET_TIME(inicio);
    multiplicaMatriz(matriz1, linhas1, colunas1, matriz2, linhas2, colunas2, resultado);
    GET_TIME(final);

    printf("Multiplicação sequencial concluída em %lf segundos\n", final - inicio);

    fwrite(&linhas1, sizeof(int), 1, arquivo_saida);
    fwrite(&colunas2, sizeof(int), 1, arquivo_saida);
    fwrite(resultado, sizeof(float), linhas1 * colunas2, arquivo_saida);

    fclose(arquivo_saida);
    free(matriz1);
    free(matriz2);
    free(resultado);

    return 0;
}

