#ifndef AGENTE_H
#define AGENTE_H

#define MAX_CUSTO_CARGA 1000000
#define MAX_AG_THREAD 1000

typedef struct {
  int x, y;   // Posições locais
  int gx, gy; // Posições globais
  double energia;
} Agente;

typedef struct {
  Agente agentes[MAX_AG_THREAD];
  int count;
} ThreadBuffer;

// Assinaturas
void executar_carga(double recurso);

#endif
