# Projeto de Implementação — Simulação Distribuída e Paralela de Mobilidade Territorial Sazonal

> Disciplina: Introdução ao Processamento Paralelo e Distribuído — 2025/2  
> Grupo: Cassiano Pessoa, Enderson Kruger, Leonardo Vieira, Luciani Aquino

---
## Divisão das tarefas....
---

## Sumário

1. [Visão Geral](#visão-geral)
2. [Compilação e Execução](#compilação-e-execução)
3. [Estrutura do Projeto](#estrutura-do-projeto)
4. [Modelo do Problema](#modelo-do-problema)
5. [Particionamento do Domínio](#particionamento-do-domínio)
6. [Tipo MPI Derivado para Agente](#tipo-mpi-derivado-para-agente)
7. [Loop Principal da Simulação](#loop-principal-da-simulação)
8. [Sazonalidade](#sazonalidade)
9. [Troca de Halo do Grid](#troca-de-halo-do-grid)
10. [Processamento de Agentes com OpenMP](#processamento-de-agentes-com-openmp)
11. [Migração de Agentes entre Processos MPI](#migração-de-agentes-entre-processos-mpi)
12. [Atualização do Grid com OpenMP](#atualização-do-grid-com-openmp)
13. [Métricas Globais](#métricas-globais)
14. [Visualização e Log](#visualização-e-log)
15. [Benchmark](#benchmark)

---

## Visão Geral

Este projeto implementa uma simulação híbrida **MPI + OpenMP** de mobilidade territorial sazonal. O território global é um grid 2D de `20 × 20` células particionado entre processos MPI ao longo do eixo Y. Dentro de cada processo, os agentes (grupos familiares) são processados em paralelo com OpenMP. Quando um agente cruza a fronteira vertical do subgrid local, ele é migrado via MPI para o processo vizinho.

A simulação roda por `100 ciclos` (`T_TOTAL`), alternando entre estações (`SECA` e `CHEIA`) a cada `10 ciclos` (`S_SAZONAL`).

---

## Compilação e Execução

```bash
# Compilar o projeto
$ make

# Executar com 4 processos MPI
$ mpirun -np 4 ./simulacao

# Gerar relatório de benchmark em .txt
$ ./benchmark.sh
```

Para controlar o número de threads OpenMP por processo, basta definir a variável de ambiente `OMP_NUM_THREADS` antes do `mpirun`. O `Makefile` compila com `mpicc`, otimização `-O2`, suporte a OpenMP via `-fopenmp` e padrão C11.

> **Atenção:** o flag `-fopenmp` é obrigatório. Sem ele, as diretivas `#pragma omp` são ignoradas silenciosamente e o código executa com uma única thread sem qualquer aviso de compilação.

---

## Estrutura do Projeto

```
.
├── README.md
├── Makefile
├── benchmark.sh
├── log.txt                  # Gerado em tempo de execução pelo rank 0
└── src/
    ├── main.c               # Ponto de entrada — inicialização, loop principal, finalização
    ├── agente.h / agente.c  # Struct Agente, ThreadBuffer, carga sintética, lógica de movimento
    ├── grid.h / grid.c      # Struct Celula, f_tipo(), f_recurso(), tipos de terreno
    ├── logger.h / logger.c  # iniciar_log(), registrar_log_ciclo()
    └── visualizacao.h / visualizacao.c  # visualizar_subgrid()
```

---

## Modelo do Problema

### Agente

Cada agente representa um grupo familiar e carrega duas posições: a **local** (`x`, `y`), relativa ao subgrid do processo que o detém, e a **global** (`gx`, `gy`), relativa ao grid completo. O estado interno é a `energia`, que começa em `100.0`.

A cada ciclo o agente perde `1.0` de energia pelo simples custo de existir, consome até `2.0` de recurso da célula ocupada (recuperando energia equivalente ao que conseguiu comer), realiza um *random walk* de ±1 em X e Y, e migra para outro processo caso cruze a fronteira Y do subgrid local.

### Célula

Cada célula possui `tipo`, `recurso` e `acessivel`. Os tipos disponíveis são `ALDEIA`, `PESCA`, `COLETA`, `ROCADO` e `INTERDITA`. O recurso inicial e o teto de regeneração dependem do tipo de terreno, definidos pela função `f_recurso()`.

### Parâmetros fixos

| Parâmetro | Valor |
|---|---|
| Grid global | 20 × 20 |
| Total de agentes | 100 |
| Ciclos totais | 100 |
| Ciclos por estação | 10 |
| Taxa de regeneração (seca) | 1.5 por ciclo |
| Taxa de regeneração (cheia) | 3.0 por ciclo |

---

## Particionamento do Domínio

O grid global é dividido ao longo do **eixo Y**. Cada processo recebe uma faixa horizontal contígua de altura `H_local = H_global / size`, mantendo a largura total do grid. Com 4 processos num grid 20×20, por exemplo, cada processo fica responsável por uma faixa de 5 linhas:

```
Grid Global 20×20
┌───────────────────┐
│   rank 0 (Y 0–4)  │  H_local = 5
├───────────────────┤
│   rank 1 (Y 5–9)  │
├───────────────────┤
│  rank 2 (Y 10–14) │
├───────────────────┤
│  rank 3 (Y 15–19) │
└───────────────────┘
```

Os agentes nascem dentro da faixa Y do processo que os criou, com semente aleatória distinta por processo para evitar que todos nasçam no mesmo lugar.

---

## Tipo MPI Derivado para Agente

Para transmitir structs `Agente` completas entre processos sem serialização manual, é registrado um **tipo MPI derivado** logo após `MPI_Init`. Ele descreve ao MPI o layout exato dos campos da struct na memória — os dois inteiros de posição local, os dois de posição global e o double de energia — usando `MPI_Type_create_struct` com os deslocamentos calculados por `offsetof`. O tipo é confirmado com `MPI_Type_commit` antes de qualquer uso e liberado com `MPI_Type_free` ao final da simulação.

---

## Loop Principal da Simulação

A cada ciclo `t`, de `0` até `T_TOTAL - 1`, os seguintes passos são executados em ordem:

- **5.1 — Atualizar estação** via `MPI_Bcast`
- **5.2 — Troca de halo do grid** via `MPI_Sendrecv` com os vizinhos de cima e de baixo
- **5.3 — Processar agentes** com região `#pragma omp parallel` / `#pragma omp for`, incluindo carga sintética, consumo atômico de recurso, random walk e separação entre agentes locais e a migrar
- **5.4 — Migrar agentes** via `MPI_Sendrecv` em duas rodadas (contagens e depois dados)
- **5.5 — Atualizar grid local** com `#pragma omp parallel for collapse(2)`
- **5.6 — Métricas globais** via `MPI_Allreduce` (três reduções: agentes, energia e recurso)
- **5.7 — Visualização** com `MPI_Barrier` e impressão sequencial por processo

---

## Sazonalidade

O rank 0 decide a alternância de estação a cada `S_SAZONAL` ciclos e difunde o novo valor para todos os processos via `MPI_Bcast`. Dessa forma, todos os processos entram no passo de atualização do grid com o mesmo estado de estação, sem necessidade de sincronização adicional.

A estação afeta diretamente a taxa de regeneração de recurso por célula: `1.5` por ciclo na estação seca e `3.0` na estação cheia. Células do tipo `INTERDITA` e `ALDEIA` não regeneram recurso em nenhuma estação.

---

## Troca de Halo do Grid

Antes de processar os agentes, cada processo precisa conhecer o estado das células imediatamente além de sua fronteira, pois um agente na borda do subgrid pode querer se mover para uma célula do vizinho antes de migrar de fato. Para isso, cada processo envia sua primeira linha ao vizinho de cima e sua última linha ao vizinho de baixo, recebendo em troca as bordas correspondentes desses vizinhos — os chamados *halos*.

A troca é feita com `MPI_Sendrecv`, que combina envio e recepção numa única chamada e evita deadlocks. Nos processos das extremidades (rank 0 sem vizinho acima, rank `size-1` sem vizinho abaixo), o destino é definido como `MPI_PROC_NULL`, valor especial que faz o MPI ignorar silenciosamente a operação — sem condicionais no código.

---

## Processamento de Agentes com OpenMP

O laço sobre os agentes locais é paralelizado com uma região `#pragma omp parallel` contendo um `#pragma omp for`. Para evitar contenção, cada thread mantém um **buffer privado** de agentes que permaneceram no subgrid local. Ao final da região paralela, esses buffers são fundidos sequencialmente na lista principal.

Agentes que cruzaram a fronteira Y são inseridos nos buffers de envio MPI dentro de uma `#pragma omp critical`, já que múltiplas threads podem identificar migrações ao mesmo tempo e os buffers são compartilhados.

O consumo de recurso da célula por múltiplos agentes em paralelo é protegido com `#pragma omp atomic`, que garante que o decremento seja atômico e livre de corrida de dados sem o custo de um lock completo.

---

## Migração de Agentes entre Processos MPI

A migração ocorre em **duas etapas** com `MPI_Sendrecv`. Na primeira, cada processo comunica ao vizinho quantos agentes irá enviar — o que permite ao receptor alocar o buffer de tamanho exato antes de receber os dados. Na segunda etapa, os agentes são transferidos de fato usando o tipo derivado `mpi_agente_type`, que encapsula toda a struct sem serialização manual.

Ao chegar no novo processo, a coordenada local Y de cada agente é ajustada para a borda correta do novo subgrid: quem veio pelo topo é posicionado na última linha local e quem veio pelo fundo é posicionado na primeira. A lista de agentes cresce dinamicamente com `realloc` caso o número de agentes recebidos exceda a capacidade alocada.

---

## Atualização do Grid com OpenMP

Após a migração, os recursos de todas as células do grid local são atualizados em paralelo com `#pragma omp parallel for collapse(2)`. A diretiva `collapse(2)` combina os dois loops aninhados (linhas e colunas) em um único espaço de iteração, aumentando a granularidade disponível ao escalonador OpenMP e evitando ociosidade de threads em grids estreitos.

O recurso de cada célula cresce pela taxa da estação vigente e é limitado ao teto definido pelo tipo de terreno, via `f_recurso()`. Células `INTERDITA` e `ALDEIA` são excluídas da regeneração.

---

## Métricas Globais

A cada ciclo, cada processo calcula localmente com OpenMP (usando `reduction`) três grandezas: total de agentes, soma de energia e soma de recursos. Em seguida, `MPI_Allreduce` com operação `MPI_SUM` agrega esses valores de todos os processos, tornando o resultado global disponível em todos simultaneamente.

O rank 0 imprime as estatísticas no terminal a cada 10 ciclos e registra uma linha por ciclo no arquivo `log.txt`.

---

## Visualização e Log

A cada ciclo a simulação exibe uma animação no terminal. Como múltiplos processos MPI escrevem no mesmo terminal, a impressão é feita **sequencialmente**: uma barreira `MPI_Barrier` sincroniza todos os processos, o rank 0 limpa o terminal, e em seguida cada processo aguarda sua vez num laço com barreira antes de chamar `visualizar_subgrid()`. Uma pausa de `400 ms` ao final de cada ciclo dá tempo suficiente para leitura humana.

O arquivo `log.txt` é criado exclusivamente pelo rank 0 via `iniciar_log()` no início da simulação, apagando execuções anteriores. A função `registrar_log_ciclo()` acrescenta uma linha a cada ciclo com o estado global da simulação.

---

## Benchmark

O script `benchmark.sh` executa a simulação com diferentes combinações de processos MPI e threads OpenMP, registrando o tempo de cada configuração em `results/benchmark.txt`. O tempo é medido internamente com `MPI_Wtime()` pelo rank 0: um `MPI_Barrier` antes do loop de simulação garante que todos os processos estejam prontos antes de o cronômetro começar, e outro `MPI_Barrier` ao final garante que todos terminaram antes de o tempo ser registrado.

```bash
$ ./benchmark.sh
```

---

# INTRODUÇÃO AO PENSAMENTO PARALELO E DISTRIBUÍDO

## Projeto de Implementação

` $ make ` para compilar o projeto

` $ mpirun -np 4 ./simulacao ` para executar a simulação

` $ ./benchmark.sh ` para gerar um .txt de benchmark
