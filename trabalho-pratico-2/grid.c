#include <stdlib.h>
#include <omp.h>
#include "grid.h"

/*
 * Funções auxiliares determinísticas descritas no Algoritmo 1 do PDF[cite: 103, 105, 107].
 * Como a semente fixa será chamada no main[cite: 28], gerar propriedades
 * baseadas em gx e gy garante que o terreno global seja sempre o mesmo,
 * independentemente de como foi fatiado pelo MPI.
 */

static TipoCelula f_tipo(int gx, int gy) {
    // Uma fórmula pseudo-aleatória simples baseada nas coordenadas
    int val = (gx * 31 + gy * 17) % 100;

    if (val < 5)  return ALDEIA;       // 5% de chance
    if (val < 15) return INTERDITADA;  // 10% de chance
    if (val < 40) return PESCA;        // 25% de chance
    if (val < 70) return COLETA;       // 30% de chance
    return ROCADO;                     // 30% de chance
}

static double f_recurso(TipoCelula tipo) {
    switch (tipo) {
        case ALDEIA:      return 0.0;
        case PESCA:       return 150.0;
        case COLETA:      return 80.0;
        case ROCADO:      return 250.0;
        case INTERDITADA: return 0.0;
        default:          return 0.0;
    }
}

static int f_acesso(TipoCelula tipo, int estacao) {
    if (tipo == INTERDITADA) return 0; // Sempre inacessível

    // Exemplo sazonal fictício: Pesca fecha na estação 1 (seca)
    if (tipo == PESCA && estacao == 1) return 0; 

    return 1;
}

/*
 * Aloca o subgrid local como um vetor 1D contíguo.
 * Facilita enormemente o empacotamento para MPI.
 */
Celula* alocar_grid_local(int largura_local, int altura_local) {
    return (Celula*) malloc(largura_local * altura_local * sizeof(Celula));
}

void liberar_grid_local(Celula* grid) {
    free(grid);
}

/*
 * Inicializa o grid local[cite: 95].
 * Converte o índice local (i, j) para o índice global (gx, gy) usando os offsets[cite: 99, 101].
 */
void inicializar_grid_local(Celula* grid, int largura_local, int altura_local, int offset_x, int offset_y, int estacao_inicial) {
    for (int i = 0; i < largura_local; i++) {
        for (int j = 0; j < altura_local; j++) {
            int gx = offset_x + i;
            int gy = offset_y + j;

            int index = i * altura_local + j; // Mapeamento 2D para 1D

            grid[index].tipo = f_tipo(gx, gy);
            grid[index].recurso = f_recurso(grid[index].tipo);
            grid[index].acessivel = f_acesso(grid[index].tipo, estacao_inicial);
        }
    }
}

/*
 * 5.5) Atualizar grid local (OpenMP) [cite: 175]
 * Paraleliza a regeneração dos recursos de todas as células[cite: 55, 178, 179].
 */
void atualizar_grid_local(Celula* grid, int largura_local, int altura_local, int estacao) {
    int total_celulas = largura_local * altura_local;

    // OpenMP irá dividir este loop for entre as threads disponíveis [cite: 176]
    #pragma omp parallel for
    for (int index = 0; index < total_celulas; index++) {

        // Atualiza a acessibilidade baseada na nova estação
        grid[index].acessivel = f_acesso(grid[index].tipo, estacao);

        // Aplica regeneração [cite: 179]
        if (grid[index].tipo != INTERDITADA && grid[index].tipo != ALDEIA) {
            double taxa_regeneracao = (estacao == 0) ? 10.0 : 5.0; // Exemplo de variação
            grid[index].recurso += taxa_regeneracao;

            // Limita o recurso a um teto máximo para não crescer infinitamente
            double teto = f_recurso(grid[index].tipo);
            if (grid[index].recurso > teto) {
                grid[index].recurso = teto;
            }
        }
    }
}
