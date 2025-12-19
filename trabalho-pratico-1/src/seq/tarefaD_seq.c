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

int main(void)
{
    double *a = malloc(N * sizeof(double));
    for (int i = 0; i < N; i++)
        a[i] = (double)(i % B);

    double sum = 0.0;

    double t0 = omp_get_wtime();
    for (int i = 0; i < N; i++)
        for (int k = 0; k < K; k++)
            sum += a[(i + k) % N];
    double t1 = omp_get_wtime();

    printf("%.6f\n", t1 - t0);
    free(a);
    return 0;
}
