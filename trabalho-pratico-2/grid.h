#ifndef GRID_H
#define GRID_H

// Tipos de área definidos no documento do projeto [cite: 21]
typedef enum {
    ALDEIA,
    PESCA,
    COLETA,
    ROCADO,
    INTERDITADA
} TipoCelula;

// Estrutura de cada célula do território [cite: 20]
typedef struct {
    TipoCelula tipo;     // aldeia, pesca, coleta, roçado ou área interditada [cite: 21]
    double recurso;      // capacidade local de sustentar agentes [cite: 22]
    int acessivel;       // depende da estação (seca ou cheia) [cite: 23]
} Celula;

// Assinaturas das funções do grid
Celula* alocar_grid_local(int largura_local, int altura_local);
void liberar_grid_local(Celula* grid);
void inicializar_grid_local(Celula* grid, int largura_local, int altura_local, int offset_x, int offset_y, int estacao_inicial);
void atualizar_grid_local(Celula* grid, int largura_local, int altura_local, int estacao);

#endif
