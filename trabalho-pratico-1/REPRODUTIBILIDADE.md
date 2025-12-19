# INTRODUÇÃO AO PENSAMENTO PARALELO E DISTRIBUÍDO

# REPRODUTIBILIDADE

## Ambiente

-   SO: Linux
-   Compilador: GCC 12.x
-   OpenMP: 5.x
-   CPU: x86_64 multicore
-   Política de afinidade: padrão do sistema

## Compilação

-   make omp N=1000000 K=20 B=256
-   export OMP_NUM_THREADS=8
-   export OMP_SCHEDULE=static,1
-   ./bin/omp

## Parâmetros

-   N ∈ {100k, 500k, 1M}
-   K ∈ {20, 24, 28}
-   B ∈ {32, 256, 4096}
-   Repetições: 5
-   Métrica: tempo total e speedup

## Observações

-   Resultados determinísticos
-   Overhead de criação de região paralela claramente observado
