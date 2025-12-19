#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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
   Variante 1 — critical
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
   Variante 2 — atomic
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
   Variante 3 — agregação local (ideal)
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

int main(void) {
    double *a = malloc(sizeof(double) * N);
    if (!a) return 1;

    double t_c, t_a, t_l;
    double s1 = variant_critical(a, N, &t_c);
    double s2 = variant_atomic(a, N, &t_a);
    double s3 = variant_local(a, N, &t_l);

    printf("%d,%d,%d,%d,%s,%f,%f,%f\n",
           N, K, B, omp_get_max_threads(),
           getenv("OMP_SCHEDULE"),
           t_c, t_a, t_l);

    free(a);
    return 0;
}
