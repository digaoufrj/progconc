#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 // valor máximo de um elemento do vetor

int main(int argc, char*argv[]) {
   if(argc < 3) {
      fprintf(stderr, "Uso: %s <dimensao N> <arquivo de saida>\n", argv[0]);
      return 1;
   }

   long int N = atol(argv[1]);
   FILE *arq_saida = fopen(argv[2], "wb");
   if(!arq_saida) {
      fprintf(stderr, "Erro ao abrir o arquivo de saida\n");
      return 2;
   }

   float *vetor_a = (float*) malloc(sizeof(float) * N);
   float *vetor_b = (float*) malloc(sizeof(float) * N);
   if(!vetor_a || !vetor_b) {
      fprintf(stderr, "Erro ao alocar memoria para os vetores\n");
      return 3;
   }

   srand(time(NULL));
   for(long int i = 0; i < N; i++) {
      vetor_a[i] = (rand() % MAX) / (float) (MAX/10);
      vetor_b[i] = (rand() % MAX) / (float) (MAX/10);
   }

   double produto_interno = 0.0;
   for(long int i = 0; i < N; i++) {
      produto_interno += vetor_a[i] * vetor_b[i];
   }

   fwrite(&N, sizeof(long int), 1, arq_saida);
   fwrite(vetor_a, sizeof(float), N, arq_saida);
   fwrite(vetor_b, sizeof(float), N, arq_saida);
   fwrite(&produto_interno, sizeof(double), 1, arq_saida);

   fclose(arq_saida);
   free(vetor_a);
   free(vetor_b);

   return 0;
}

