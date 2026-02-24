#include <stdio.h>
#include <mpi.h>
#include "visualizacao.h"

#define RESET  "\x1B[0m"
#define RED    "\x1B[31m"
#define GRN    "\x1B[32m"
#define BLU    "\x1B[34m"
#define YEL    "\x1B[33m"
#define MAG    "\x1B[35m"
#define CYAN   "\x1B[36m"
#define GRY    "\x1B[90m"

void visualizar_subgrid(int rank, int W_local, int H_local, int offsetY, Celula* grid, Agente* agentes, int n_agentes) {
    MPI_Barrier(MPI_COMM_WORLD); 

    printf("\n--- Processo MPI Rank %d (Linhas Globais: %d a %d) ---\n", rank, offsetY, offsetY + H_local - 1);

    for (int j = 0; j < H_local; j++) {
        for (int i = 0; i < W_local; i++) {
            int idx = j * W_local + i;
            bool tem_agente = false;

            for (int k = 0; k < n_agentes; k++) {
                if (agentes[k].x == i && agentes[k].y == j) {
                    tem_agente = true;
                    break;
                }
            }

            if (tem_agente) {
                printf(RED " @ " RESET);
            } else {
                switch (grid[idx].tipo) {
                    case ALDEIA:    printf(MAG " H " RESET); break;
                    case PESCA:     printf(BLU " ~ " RESET); break;
                    case COLETA:    printf(GRN " f " RESET); break;
                    case ROCADO:    printf(YEL " # " RESET); break;
                    case INTERDITA: printf(" X "); break;
                }
            }
        }
        printf("\n");
    }
    fflush(stdout);
}
