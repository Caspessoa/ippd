#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#ifndef N
#define N 1000000
#endif

#ifndef K
#define K 20
#endif

#ifndef B
#define B 256
#endif

/* =========================================================
   Variante 0 — Naive (Tarefa D: Variante Ingênua)
   Dois 'parallel for' consecutivos.
   Causa overhead de barreira implícita e gestão de threads 2x.
   ========================================================= */
double variant_naive(double *a, int n, double *t) {
    double t0 = omp_get_wtime();
    double sum = 0.0;

    // Kernel 1: Inicialização
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < n; i++)
        a[i] = (double)(i % B) * 0.5;

    // Kernel 2: Processamento
    #pragma omp parallel for schedule(runtime) reduction(+:sum)
    for (int i = 0; i < n; i++)
        sum += a[i] * (double)K;

    *t = omp_get_wtime() - t0;
    return sum;
}

/* =========================================================
   Variante 1 — Critical (Tarefa D: Variante Arrumada + Critical)
   ========================================================= */
double variant_critical(double *a, int n, double *t) {
    double t0 = omp_get_wtime();
    double sum = 0.0;

    #pragma omp parallel
    {
        double local = 0.0;

        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++)
            a[i] = (double)(i % B) * 0.5;

        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++)
            local += a[i] * (double)K;

        #pragma omp critical
        sum += local;
    }

    *t = omp_get_wtime() - t0;
    return sum;
}

/* =========================================================
   Variante 2 — Atomic (Requisito 7.4)
   ========================================================= */
double variant_atomic(double *a, int n, double *t) {
    double t0 = omp_get_wtime();
    double sum = 0.0;

    #pragma omp parallel
    {
        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++)
            a[i] = (double)(i % B) * 0.5;

        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++) {
            double tmp = a[i] * (double)K;
            #pragma omp atomic
            sum += tmp;
        }
    }

    *t = omp_get_wtime() - t0;
    return sum;
}

/* =========================================================
   Variante 3 — Local Aggregation (Tarefa D: Variante Arrumada Ideal)
   ========================================================= */
double variant_local(double *a, int n, double *t) {
    double t0 = omp_get_wtime();
    double sum = 0.0;

    #pragma omp parallel
    {
        double local = 0.0;

        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++)
            a[i] = (double)(i % B) * 0.5;

        #pragma omp for schedule(runtime)
        for (int i = 0; i < n; i++)
            local += a[i] * (double)K;

        #pragma omp atomic
        sum += local;
    }

    *t = omp_get_wtime() - t0;
    return sum;
}

/* =========================================================
   Variante 4 — SIMD (Requisito 7.5)
   Baseada na variant_local, mas forçando vetorização.
   CORREÇÃO: reduction aplicada diretamente em 'sum'.
   ========================================================= */
double variant_simd(double *a, int n, double *t) {
    double t0 = omp_get_wtime();
    double sum = 0.0;

    #pragma omp parallel
    {
        // O compilador tentará vetorizar a inicialização
        #pragma omp for simd schedule(runtime)
        for (int i = 0; i < n; i++)
            a[i] = (double)(i % B) * 0.5;

        // CORREÇÃO AQUI: 
        // Usamos reduction(+:sum) em vez de reduction(+:local).
        // O OpenMP cria automaticamente uma cópia privada de 'sum' para cada thread
        // (comportamento de agregação local) e aplica instruções SIMD na soma.
        #pragma omp for simd reduction(+:sum) schedule(runtime)
        for (int i = 0; i < n; i++)
            sum += a[i] * (double)K;
    }

    *t = omp_get_wtime() - t0;
    return sum;
}

int main(void) {
    double *a = malloc(sizeof(double) * N);
    if (!a) return 1;

    double t_n, t_c, t_a, t_l, t_s;
    
    // Executa todas as variantes
    double s0 = variant_naive(a, N, &t_n);
    double s1 = variant_critical(a, N, &t_c);
    double s2 = variant_atomic(a, N, &t_a);
    double s3 = variant_local(a, N, &t_l);
    double s4 = variant_simd(a, N, &t_s);
    
    char *env_sched = getenv("OMP_SCHEDULE");
    char sched_buffer[64];
    if (env_sched) {
        snprintf(sched_buffer, 64, "%s", env_sched);
        for (int i = 0; sched_buffer[i]; i++) {
            if (sched_buffer[i] == ',') sched_buffer[i] = '-';
        }
    } else {
        snprintf(sched_buffer, 64, "default");
    }

    printf("%d,%d,%d,%d,%s,%f,%f,%f,%f,%f\n",
           N, K, B, omp_get_max_threads(),
           sched_buffer, // Usa a versão higienizada (static-64)
           t_n, t_c, t_a, t_l, t_s);

    free(a);
    return 0;
}