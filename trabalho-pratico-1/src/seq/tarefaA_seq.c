#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Usando time.h ou outra API de tempo compatível com C para referência

// Função Fibonacci recursiva (custosa propositalmente)
long long fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

// Função para medir o tempo (Substitua por omp_get_wtime() no ambiente final se preferir)
double medir_tempo_inicio() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

double medir_tempo_fim(double inicio) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double fim = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    return fim - inicio;
}

int main(int argc, char *argv[]) {
    // --------------------------------------------------------
    // Apenas a Tarefa A precisa de N e K
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <N> <K>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    // --------------------------------------------------------

    long long *v = (long long *)malloc(N * sizeof(long long));
    if (v == NULL) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        return 1;
    }

    double inicio = medir_tempo_inicio();

    // Laço sequencial
    for (int i = 0; i < N; i++) {
        v[i] = fib(i % K);
    }

    double tempo_total = medir_tempo_fim(inicio);

    // Saída CSV simples: Tempo
    // O script Python deve rodar com 1 thread para ter uma base sequencial justa
    printf("%f", tempo_total);

    free(v);
    return 0;
}