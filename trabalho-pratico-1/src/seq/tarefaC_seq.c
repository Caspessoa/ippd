#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void gerar_dados(double *v, int N) {
    for (int i = 0; i < N; i++) {
        v[i] = (double)(rand() % 100) / 10.0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    double a = 2.5; // Fator escalar constante

    // Alocação alinhada (opcional, mas ajuda o SIMD se fosse usado aqui)
    double *x = (double*) malloc(N * sizeof(double));
    double *y = (double*) malloc(N * sizeof(double));

    gerar_dados(x, N);
    gerar_dados(y, N);

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // V1: Sequencial (O compilador será forçado a NÃO vetorizar aqui via Makefile)
    for (int i = 0; i < N; i++) {
        y[i] = a * x[i] + y[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("%f", tempo);

    free(x);
    free(y);
    return 0;
}