#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N 1000000
#endif

#ifndef K
#define K 20
#endif

#ifndef B
#define B 256
#endif

int main(void) {
    double *a = malloc(sizeof(double) * N);
    double sum = 0.0;

    for (int i = 0; i < N; i++)
        a[i] = (double)(i % B) * 0.5;

    for (int i = 0; i < N; i++)
        sum += a[i] * (double)K;

    printf("%d,%d,%d,SEQ,%f\n", N, K, B, sum);
    free(a);
    return 0;
}
