# INTRODUÇÃO AO PENSAMENTO PARALELO E DISTRIBUÍDO

# REPRODUTIBILIDADE

Para garantir a reprodução dos resultados apresentados, detalhamos abaixo o ambiente de hardware e software, bem como as configurações de compilação utilizadas.

## Ambiente

-   SO: Linux
-   Compilador: GCC 12.x
-   OpenMP: 5.x
-   CPU: x86_64 multicore
-   Política de afinidade: padrão do sistema

## Compilação

-   `$ make`
-   `$ make run`

## Parâmetros

-   N ∈ {100k, 500k, 1M}
-   K ∈ {20, 24, 28}
-   B ∈ {32, 256, 4096}
-   Repetições: 5
-   Métrica: tempo total e speedup

## Compilação

O projeto utiliza um `Makefile` para padronizar as flags.

-   **Flags de Otimização:** `-O3` (Habilita otimizações agressivas e vetorização automática básica)
-   **Flags OpenMP:** `-fopenmp`
-   **Flags de Aviso:** `-Wall`

## Observações

-   Resultados determinísticos
-   Overhead de criação de região paralela claramente observado
