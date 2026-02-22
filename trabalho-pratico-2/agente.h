#ifndef AGENTE_H
#define AGENTE_H

// Estrutura sugerida pelo professor
typedef struct {
    int x;
    int y;
    double energia;
} Agente;

// Assinaturas das funções relacionadas ao agente
void executar_carga(double recurso);
void consumir_recurso(double *recurso_celula, double quantidade);
void decidir_movimento(Agente *a, int max_x, int max_y);

#endif
