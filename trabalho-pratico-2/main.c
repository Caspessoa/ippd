#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Definição dos tipos de terreno
typedef enum {
    ALDEIA,
    PESCA,
    COLETA,
    ROCADO,
    INTERDITA
} TipoCelula;

// Controle de sazonalidade 
typedef enum {
    SECA,
    CHEIA
} Estacao;

// Estrutura da Célula do Grid 
typedef struct {
    TipoCelula tipo;     // Tipo do território
    double recurso;      // Capacidade de sustentar agentes
    bool acessivel;      // Depende da estação atual
} Celula;

// Estrutura do Agente (Grupo Familiar)
typedef struct {
    int x, y;            // Posição no grid global/local 
    double energia;      // Estado interno ou necessidade
} Agente;

// Constante para limitar o custo da carga sintética 
#define MAX_CUSTO_CARGA 1000000
