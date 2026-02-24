#ifndef VISUALIZACAO_H
#define VISUALIZACAO_H

#include "agente.h"
#include "grid.h"

void visualizar_subgrid(int rank, int W_local, int H_local, int offsetY,
                        Celula *grid, Agente *agentes, int n_agentes);

#endif
