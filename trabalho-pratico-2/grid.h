#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

typedef enum { ALDEIA, PESCA, COLETA, ROCADO, INTERDITA } TipoCelula;

typedef enum { SECA, CHEIA } Estacao;

typedef struct {
  TipoCelula tipo;
  double recurso;
  bool acessivel;
} Celula;

// Assinaturas das funções
TipoCelula f_tipo(int gx, int gy);
double f_recurso(TipoCelula tipo);

#endif
