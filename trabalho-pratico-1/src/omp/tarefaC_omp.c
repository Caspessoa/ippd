#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void gerar_dados(double *v, int N) {
    for (int i = 0; i < N; i++) {
        v[i] = (double)(rand() % 100) / 10.0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <N> <Variante>\n", argv[0]);
        fprintf(stderr, "Variante: 2=SIMD, 3=Parallel SIMD\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int variante = atoi(argv[2]);
    double a = 2.5;

    double *x = (double*) malloc(N * sizeof(double));
    double *y = (double*) malloc(N * sizeof(double));

    gerar_dados(x, N);
    gerar_dados(y, N);

    double inicio = omp_get_wtime();

    if (variante == 2) {
        // V2: Apenas SIMD (uma thread, instruções vetoriais)
        // 'simd' instrui o compilador a usar registradores largos (AVX/SSE)
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            y[i] = a * x[i] + y[i];
        }
    } 
    else if (variante == 3) {
        // V3: Threads + SIMD
        // Divide o loop entre threads E vetoriza cada pedaço
        #pragma omp parallel for simd
        for (int i = 0; i < N; i++) {
            y[i] = a * x[i] + y[i];
        }
    }

    double fim = omp_get_wtime();
    printf("%f", fim - inicio);

    free(x);
    free(y);
    return 0;
}