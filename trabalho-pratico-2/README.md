# Projeto de Implementação — Simulação Distribuída e Paralela de Mobilidade Territorial Sazonal

> Disciplina: Introdução ao Processamento Paralelo e Distribuído — 2025/2  
> Grupo: Cassiano Pessoa, Enderson Kruger, Leonardo Vieira, Luciani Aquino

---
## Divisão das tarefas....
---

## Compilação e Execução

```bash
$ make
$ mpirun -np 4 ./simulacao
$ OMP_NUM_THREADS=4 mpirun -np 4 ./simulacao
```

## Rodar benchmark, uma das opções abaixo
```bash
ambiente git bash

$ ./benchmark.sh

ambiente wsl (corrigir quebra de linha)

$ sed -i 's/\r$//' benchmark.sh
$ sed -i 's/\r$//' Makefile
$ bash benchmark.sh

```
---

## O Problema

Um grid 2D de `20 × 20` células é dividido entre processos MPI. Cada célula tem um tipo (`ALDEIA`, `PESCA`, `COLETA`, `ROCADO`, `INTERDITA`) e um valor de recurso. Agentes representam grupos familiares que se movem pelo território, consomem recursos e, quando cruzam a fronteira do subgrid local, são transferidos para o processo vizinho.

A simulação roda por 100 ciclos, com a estação (`SECA`/`CHEIA`) alternando a cada 10 ciclos.

---

## Estrutura

```
src/
├── main.c               # loop principal
├── agente.h / agente.c  # struct Agente, movimento, carga sintética
├── grid.h / grid.c      # struct Celula, tipos de terreno
├── logger.h / logger.c  # log.txt por ciclo (rank 0)
└── visualizacao.h / visualizacao.c
```

---

## Paralelismo

### Distribuição do domínio com MPI

O grid é fatiado horizontalmente: cada processo recebe `H_local = H_global / size` linhas, mantendo a largura total.

```
┌──────────────────┐
│  rank 0 (Y 0–4)  │
├──────────────────┤
│  rank 1 (Y 5–9)  │
├──────────────────┤
│ rank 2 (Y 10–14) │
├──────────────────┤
│ rank 3 (Y 15–19) │
└──────────────────┘
```

Para transferir agentes entre processos sem serialização manual, é criado um **tipo MPI derivado** com `MPI_Type_create_struct`, descrevendo o layout exato da struct na memória. O tipo é registrado com `MPI_Type_commit` e liberado ao fim com `MPI_Type_free`.

### Processamento de agentes com OpenMP

O laço de agentes roda dentro de um `#pragma omp parallel for`. Cada thread mantém um **buffer privado** para os agentes que permanecem locais, eliminando contenção. Ao fim da região paralela, os buffers são fundidos na lista principal.

O consumo de recurso na célula é protegido com `#pragma omp atomic` — mais leve que um lock, suficiente para o decremento escalar. Agentes que saem do subgrid são enfileirados para envio MPI dentro de `#pragma omp critical`.

### Atualização do grid com OpenMP

Os recursos de todas as células são atualizados com `#pragma omp parallel for collapse(2)`, que combina os dois loops (linhas × colunas) em um único espaço de iteração paralelo. O valor cresce pela taxa da estação e é limitado ao teto do tipo de terreno. Células `ALDEIA` e `INTERDITA` não regeneram.

---

## Comunicações MPI por ciclo

| Passo | Operação | Descrição |
|---|---|---|
| Estação | `MPI_Bcast` | rank 0 difunde a estação atual |
| Halo | `MPI_Sendrecv` ×2 | troca das bordas superior e inferior com vizinhos |
| Migração | `MPI_Sendrecv` ×4 | primeiro as contagens, depois os agentes |
| Métricas | `MPI_Allreduce` ×3 | soma global de agentes, energia e recurso |
| Visualização | `MPI_Barrier` | impressão sequencial por processo |

A troca de halo garante que agentes próximos à fronteira consultem células do vizinho antes de decidir migrar. Processos nas extremidades usam `MPI_PROC_NULL` para dispensar condicionais de borda. A migração ocorre em duas rodadas: primeiro cada processo informa quantos agentes enviará (para o receptor alocar o buffer correto), depois os dados são transferidos com o tipo derivado.

---

## Benchmark

`benchmark.sh` roda a simulação em combinações variadas de processos MPI e threads OpenMP, gravando o tempo de cada configuração em `results/benchmark.txt`. O tempo é medido com `MPI_Wtime()`, entre dois `MPI_Barrier` — um antes e outro depois do loop principal.