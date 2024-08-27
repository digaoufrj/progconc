#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 

int main(int argc, char*argv[]) {
   if(argc < 3) {
      fprintf(stderr, "Digite: %s (dimensao) (arquivo)\n", argv[0]);
      return 1;
   }

   long int N = atol(argv[1]);
   FILE *arquivo = fopen(argv[2], "wb");
   if(!arquivo) {
      fprintf(stderr, "Erro ao abrir o arquivo\n");
      return 2;
   }

   float *vet1 = (float*) malloc(sizeof(float) * N);
   float *vet2 = (float*) malloc(sizeof(float) * N);
   if(!vet1 || !vet2) {
      fprintf(stderr, "Erro ao alocar memoria para os vetores\n");
      return 3;
   }

   srand(time(NULL));
   for(long int i = 0; i < N; i++) {
      vet1[i] = (rand() % MAX) / (float) (MAX/10);
      vet2[i] = (rand() % MAX) / (float) (MAX/10);
   }

   double produto_interno = 0.0;
   for(long int i = 0; i < N; i++) {
      produto_interno += vet1[i] * vet2[i];
   }

   fwrite(&N, sizeof(long int), 1, arquivo);
   fwrite(vet1, sizeof(float), N, arquivo);
   fwrite(vet2, sizeof(float), N, arquivo);
   fwrite(&produto_interno, sizeof(double), 1, arquivo);

   fclose(arquivo);
   free(vet1);
   free(vet2);

   return 0;
}
