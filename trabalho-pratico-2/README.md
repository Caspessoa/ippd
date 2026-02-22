# Roteiro de Implementação (Sprints)

Para organizar o desenvolvimento, dividi o trabalho em 5 etapas lógicas:

#### 1. Estruturas de Dados e Lógica Sequencial (Baseline)

O objetivo aqui é definir como os dados vivem na memória antes de pensar em paralelismo.

* 
**Definição do Agente:** Criar a `struct` para os agentes (posições , energia/estado).


* 
**Célula do Grid:** Implementar a estrutura da célula com tipo, recurso e acessibilidade.


**Carga Sintética:** Implementar a função `executar_carga(r)` que realiza cálculos inúteis para simular esforço computacional proporcional ao recurso local.


#### 2. Decomposição de Domínio (MPI Setup)

Focar na divisão do território entre os processos.

**Particionamento:** Dividir o grid global  em subgrids locais para cada processo MPI.

**Mapeamento de Coordenadas:** Implementar a lógica de *offsets* para converter coordenadas locais em globais e vice-versa.

**Inicialização Determinística:** Garantir que cada processo use uma semente fixa para que o grid seja gerado de forma consistente em todos os nós.


#### 3. Paralelismo Intra-nó (OpenMP)

Acelerar o processamento dentro de cada subgrid.

**Processamento de Agentes:** Usar `#pragma omp parallel for` para iterar sobre a lista de agentes locais, executando a tomada de decisão e a carga sintética.

**Atualização do Território:** Paralelizar a regeneração de recursos nas células usando `collapse(2)` para percorrer o grid local.


* 
**Gerenciamento de Threads:** Implementar buffers por thread para evitar condições de corrida ao adicionar/remover agentes da lista local.



#### 4. Comunicação e Migração (MPI Sync)

A parte mais crítica: a troca de informações entre processos.

* 
**Troca de Halo (Bordas):** Implementar o envio/recebimento das células de borda para que os agentes nas extremidades consigam "enxergar" o vizinho antes de decidir se mudam.


* 
**Migração de Agentes:** Desenvolver a lógica para identificar agentes que saíram do subgrid, serializá-los em buffers e enviá-los ao processo destino.

**Sincronização de Estações:** Usar `MPI_Bcast` para garantir que todos os processos mudem de estação (seca/cheia) simultaneamente.

####5. Métricas Globais e Validação
Finalização e análise de desempenho.

**Redução de Dados:** Usar `MPI_Allreduce` para consolidar estatísticas (ex: consumo total de recursos, número total de agentes vivos).

**Validação de Escalabilidade:** Testar se o tempo de execução diminui conforme você aumenta o número de processos (MPI) e threads (OpenMP).
