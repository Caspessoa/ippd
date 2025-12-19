# INTRODUÇÃO AO PENSAMENTO PARALELO E DISTRIBUÍDO

## TRABALHO PRÁTICO 1: OPENMP

Atividade extra-classe: OpenMP na prática, com ênfase em laços, escalonamento e seções críticas

### 1. Objetivo

Implementar e avaliar versões paralelas em C ou C++ usando OpenMP, com foco em:

1. Paralelização de laços e escolhas de `schedule` e `chunk`
2. Substituição criteriosa de `critical` por `atomic`
3. Uso correto de `simd` quando houver vetorizaçao possível
4. Organização da região paralela evitando paralelismo aninhado desnecessário

### 2. Formação de grupos

Integrantes & Responsabilidades:

-   [Cassiano Teixeira Pessoa](https://github.com/Caspessoa)
    -   Implentação da tarefa D
-   [Enderson Avila Kruger](https://github.com/endersonkruger)
    -   Implementação da tarefa C
-   [Leonardo Vieira Gonçalves](https://github.com/LeonardVG)
    -   Implementação da tarefa A
-   [Luciani Aquino Queiroz](https://github.com/aquinoluci)
    -   Implementação da tarefa B

---

-   Geral:
    -   Documentação
    -   Criação de ambientes de testes
    -   Análise de cenários
    -   Medições

### 3. Pré-requisitos

-   Compilar e executar C ou C++ com OpenMP
-   Medir tempo de execução e interpretar variância
-   Noções de contenção e balanceamento de carga

### 4. Ambiente

=== SISTEMA ===
Linux USER 6.6.87.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun 5 18:30:46 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux

=== COMPILADOR GCC ===
gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0

=== MAKE ===
GNU Make 4.3

=== PYTHON ===
Python 3.12.3

=== OPENMP (Verificação Básica) ===
#define \_OPENMP 201511

-   GCC ou Clang com OpenMP 5.x
-   Linux
-   `Makefile` com alvos: `seq`, `omp`, `run`, `plot`

### 5. Descrição das tarefas

#### Tarefa A — Laço irregular e políticas de `schedule`

-   Kernel: para `i = 0..N-1`, compute `fib(i % K)` e grave em `v[i]` usando _fib_ custosa sem memoização.
-   Variante 1: `#pragma omp parallel for schedule(static)`
-   Variante 2: `schedule(dynamic,chunk)` com `chunk ∈ {1,4,16,64}`
-   Variante 3: `schedule(guided,chunk)` com `chunk ∈ {1,4,16,64}`
-   Se houver dois laços paralelos em sequência, use uma única região `parallel` e dois `for` internos

#### Tarefa B — Seção crítica vs `atomic` e agregação por thread

-   Histogramas: entrada `A` em `[0,B)`, saída `H[B]`.
-   V1: atualizar `H[A[i]]++` dentro de `#pragma omp critical`.
-   V2: substituir por `#pragma omp atomic` quando válido.
-   V3: reduzir contenção com histogramas locais por thread e redução manual.
-   Comparar tempos e escalabilidade.

#### Tarefa C — Vetorização com `simd`

-   SAXPY: `y[i] = a\*x[i] + y[i]`.
-   V1: sequencial.
-   V2: `#pragma omp simd`.
-   V3: `#pragma omp parallel for simd`.
-   Analisar ganhos e limitações.

#### Tarefa D — Organização de região paralela

-   Variante ingênua: dois `parallel for` consecutivos.
-   Variante arrumada: uma região `parallel` com dois `for`.
-   Comparar overhead e tempos.

### 6. Conjuntos de testes e parâmetros

-   `N ∈ {100000, 500000, 1000000}`
-   `K ∈ {20, 24, 28}`
-   `B ∈ {32, 256, 4096}`
-   Threads: `{1, 2, 4, 8, 16}`
-   Cada ponto: média de 5 execuções, reportar desvio padrão

### 7. Medições obrigatórias

-   Tempo total e tempo dos kernels
-   Escalabilidade por número de threads
-   Impacto de `schedule` e `chunk`
-   `critical` vs `atomic` vs agregação local
-   Ganho de `simd` sobre a versão base

Scripts exigidos:

-   `run.sh`: executa a matriz de experimentos e grava CSV
-   `plot.py`: gera gráficos a partir do CSV (pode usar outra coisa que não seja Python, mas lembre que não tenho outras coisas instaladas na minha máquina e potencialmente terão que me apresentar)

### 8. Critérios para usar `atomic` em vez de `critical`

-   Atualização atômica sobre um único escalar endereçável
-   Sem efeitos colaterais na expressão de atualização
-   Se não for possível, justificar `critical` ou usar agregação por thread

### 9. Entregáveis

Deve ser submetido o link para o repositório no GitHub com o projeto desenvolvido.

Neste repositório deve ter:

1. **[README.md](README.md)** com o nome dos membros do grupo e suas responsabilidades no projeto, além de todas as orientações para compilar e executar os programas gerados.
2. Código em [src/seq](src/seq) e [src/omp](src/omp), [Makefile](makefile), [run.sh](run.sh), [plot.py](plot.py)
3. **[RESULTADOS.md](RESULTADOS.md)**: tabelas e gráficos, decisões de `schedule`, análise curta e objetiva
4. **[REPRODUTIBILIDADE.md](REPRODUTIBILIDADE.md)**: versão do compilador, flags, CPU, afinidade, semente do gerador

### 10. Avaliação

| Critério                                                | Peso |
| ------------------------------------------------------- | :--: |
| Correção funcional e reprodutibilidade                  | 30%  |
| Implementação paralela e organização da região paralela | 25%  |
| Análise de desempenho com métricas e leitura crítica    | 30%  |
| Qualidade do repositório e automação                    | 10%  |

Penalizações: medições ruidosas sem controle, gráficos sem variância, conclusões sem dados.

### 11. Observações finais

-   Compare versões com dados, não por fé
-   Prefira soluções simples e auditáveis antes de complexidade
