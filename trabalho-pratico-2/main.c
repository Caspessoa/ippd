#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "grid.h"
#include "agente.h"

// Parâmetros globais da simulação
#define W_GLOBAL 1000
#define H_GLOBAL 1000
#define CICLOS_T 100
#define CICLO_SAZONAL_S 25
#define N_AGENTES_GLOBAL 5000

int main(int argc, char** argv) {
    int rank, size;

    // 1) Inicializar MPI [cite: 78, 80]
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // [cite: 82]
    MPI_Comm_size(MPI_COMM_WORLD, &size); // [cite: 84]

    // Validação da simplificação do PDF 
    if (H_GLOBAL % size != 0) {
        if (rank == 0) {
            printf("Erro: A altura global (%d) deve ser múltipla do número de processos (%d).\n", H_GLOBAL, size);
        }
        MPI_Finalize();
        return 1;
    }

    // 2) Particionar o grid global (Fatiamento Horizontal 1D) [cite: 88, 93, 94]
    int w_local = W_GLOBAL;
    int h_local = H_GLOBAL / size;
    int offset_x = 0;
    int offset_y = rank * h_local;

    if (rank == 0) {
        printf("--- Iniciando Simulação Híbrida MPI + OpenMP ---\n");
        printf("Processos MPI: %d\n", size);
        printf("Threads OpenMP (max por nó): %d\n", omp_get_max_threads());
        printf("Dimensões Globais: %dx%d\n", W_GLOBAL, H_GLOBAL);
        printf("Dimensões Locais (por processo): %dx%d\n\n", w_local, h_local);
    }

    // 3) Inicializar grid local (determinístico) [cite: 95]
    int estacao_atual = 0;
    Celula* grid_local = alocar_grid_local(w_local, h_local);
    inicializar_grid_local(grid_local, w_local, h_local, offset_x, offset_y, estacao_atual);

    // 4) Inicializar agentes locais [cite: 110]
    // Provisório: Divide os agentes igualmente. 
    // Na versão final, sortearemos as posições e o processo só aloca se cair no seu subgrid[cite: 112].
    int n_agentes_locais = N_AGENTES_GLOBAL / size; 

    // 5) Loop principal da simulação [cite: 115]
    for (int t = 0; t < CICLOS_T; t++) {

        // 5.1) Atualizar estação sazonal [cite: 124]
        if (t > 0 && t % CICLO_SAZONAL_S == 0) {
            estacao_atual = (estacao_atual == 0) ? 1 : 0; // Alterna a estação [cite: 125]
            if (rank == 0) printf("Ciclo %d: Mudança de estação para %d!\n", t, estacao_atual);
            // Como todos os processos calculam 't' igualmente, o MPI_Bcast não é estritamente 
            // necessário aqui, mas mantém a lógica sincronizada.
            MPI_Bcast(&estacao_atual, 1, MPI_INT, 0, MPI_COMM_WORLD); // [cite: 126]
        }

        // 5.2) Troca de halo do grid (MPI) [cite: 127, 128]
        // TODO: MPI_Sendrecv das linhas '0' e 'h_local-1' para os processos vizinhos.

        // 5.3) Processar agentes (OpenMP) [cite: 154, 155]
        // TODO: #pragma omp parallel for processando agentes locais e gerando buffers de saída.

        // 5.4) Migrar agentes (MPI) [cite: 171, 172, 173]
        // TODO: Enviar agentes que saíram do subgrid local para rank-1 (cima) ou rank+1 (baixo).

        // 5.5) Atualizar grid local (OpenMP) [cite: 175]
        atualizar_grid_local(grid_local, w_local, h_local, estacao_atual);

        // 5.6) Métricas globais (MPI) [cite: 204]
        // TODO: MPI_Reduce para somar estatísticas, como recurso total restante.

        // 5.7) Sincronização opcional (Barreira) [cite: 197, 206]
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0) printf("\nSimulação concluída com sucesso.\n");

    // 6) Finalizar MPI e liberar memória [cite: 201, 208]
    liberar_grid_local(grid_local);
    MPI_Finalize();
    return 0;
}
