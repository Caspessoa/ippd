#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


/*
Definindo como a informação será organizada. 
Note que a Célula armazena o estado do território, 
enquanto o Agente representa a entidade dinâmica. 
*/
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

//-------------------------------------------------------------------------------

/**
 A função executar_carga é crucial para validar o desempenho do OpenMP no futuro. 
 Ela simula o "gasto de tempo" que o computador teria processando decisões complexas 
 baseadas nos recursos da célula
 */
void executar_carga(double recurso) {
    // O custo deve ser limitado por um valor máximo pré-definido 
    long iteracoes = (long)(recurso * 1000); // Exemplo de escala de peso
    
    if (iteracoes > MAX_CUSTO_CARGA) {
        iteracoes = MAX_CUSTO_CARGA;
    }

    // Laço de iterações com operações aritméticas simples
    volatile double dummy = 0.0;
    for (long c = 0; c < iteracoes; c++) {
        dummy += (c * 0.0001); // Operação para evitar otimização do compilador
    }
}
//---------------------------------------------------------------------------------------

/**
 * Simula a interação do agente com a célula onde ele se encontra.
 */
void processar_agente_local(Agente *a, Celula *grid_local, int largura_local) {
    // Identifica a célula atual no array linear (se for grid 2D linearizado)
    int idx = a->y * largura_local + a->x;
    Celula *c = &grid_local[idx];

    // Executa a carga sintética baseada no recurso da célula 
    executar_carga(c->recurso);

    // Agente consome parte do recurso da célula 
    double consumo = 10.0; // Valor hipotético de consumo
    if (c->recurso >= consumo) {
        c->recurso -= consumo; 
        a->energia += consumo;
    } else {
        a->energia += c->recurso;
        c->recurso = 0;
    }
}



//====================================================================
//                           MAIN
//====================================================================

int main(int argc, char** argv) {
    // 4. Boilerplate MPI (Ainda não fizemos)
    // MPI_Init, MPI_Comm_rank, etc. 
    return 0;
}