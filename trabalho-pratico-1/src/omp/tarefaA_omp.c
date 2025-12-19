#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

// Função Fibonacci recursiva (custosa propositalmente)
long long fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv[]) {
    // --------------------------------------------------------
    // Argumentos: N, K, Schedule_ID, Chunk_size
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <N> <K> <schedule_type_id> <chunk_size>\n", argv[0]);
        fprintf(stderr, "schedule_type_id: 0=static, 1=dynamic, 2=guided\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    int sched_type_in = atoi(argv[3]); 
    int chunk_size = atoi(argv[4]);
    // --------------------------------------------------------

    long long *v = (long long *)malloc(N * sizeof(long long));
    if (v == NULL) {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 1;
    }

    // Configuração do Schedule via API do OpenMP
    omp_sched_t tipo_sched;
    switch (sched_type_in) {
        case 0: tipo_sched = omp_sched_static; break;
        case 1: tipo_sched = omp_sched_dynamic; break;
        case 2: tipo_sched = omp_sched_guided; break;
        default: tipo_sched = omp_sched_static;
    }
    
    // Define a política e o chunk para a próxima região com 'runtime'
    omp_set_schedule(tipo_sched, chunk_size);

    double inicio = omp_get_wtime();

    // Região paralela
    #pragma omp parallel
    {
        // Laço principal com escalonamento definido em runtime
        #pragma omp for schedule(runtime)
        for (int i = 0; i < N; i++) {
            v[i] = fib(i % K);
        }
        
        // Se houvesse um segundo laço, ele iria aqui, dentro da mesma região parallel
        /*
        #pragma omp for schedule(runtime)
        for (int i = 0; i < N; i++) {
            // Outro trabalho...
        }
        */
    } // Fim da região parallel

    double fim = omp_get_wtime();
    double tempo_total = fim - inicio;

    // Saída CSV simples: Tempo
    printf("%f", tempo_total);

    free(v);
    return 0;
}