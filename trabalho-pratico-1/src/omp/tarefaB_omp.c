#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// Programa para computar histograma com diferentes estratégias de sincronização
// comparando o critical e o atomic
/// critical: diretiva para proteger uma seção crítica uma thread por vez
// atomic: diretiva para operações atômicas em variáveis compartilhadas

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <N> <B> <versao>\n", argv[0]); // N: tamanho do array, B: número de bins, versao: 1, 2 ou 3
        fprintf(stderr, "versao: 1=Critical, 2=Atomic, 3=Local+Reduction\n");
        return 1;
    }

    int N = atoi(argv[1]); // Tamanho do array de entrada
    int B = atoi(argv[2]); // Número de bins (intervalo [0, B))
    int versao = atoi(argv[3]);

    // Alocação de memória
    int *A = (int *)malloc(N * sizeof(int));
    long long *H = (long long *)calloc(B, sizeof(long long));

    // Inicialização do array com valores aleatórios entre 0 e B-1
    srand(42); // Reprodutibilidade
    for (int i = 0; i < N; i++) {
        A[i] = rand() % B;
    }

    double inicio = omp_get_wtime();

    if (versao == 1) {
        // V1: Seção Crítica (Alta Contenção)
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            #pragma omp critical
            {
                H[A[i]]++;
            }
        }
    } else if (versao == 2) {
        // V2: Atômico (Baixa Contenção)
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            H[A[i]]++;
        }
    } else if (versao == 3) {
        // V3: Histogramas Locais e Redução Manual
        #pragma omp parallel
        {
            // Cada thread cria seu próprio histograma local
            long long *local_H = (long long *)calloc(B, sizeof(long long));

            #pragma omp for
            for (int i = 0; i < N; i++) {
                local_H[A[i]]++;
            }

            // Redução manual dos resultados locais para o global
            for (int j = 0; j < B; j++) {
                #pragma omp atomic
                H[j] += local_H[j];
            }
            free(local_H);
        }
    }

    double fim = omp_get_wtime();
    // Saída CSV simples: Tempo
    printf("%f\n", fim - inicio);


    free(A);
    free(H);
    return 0;
}