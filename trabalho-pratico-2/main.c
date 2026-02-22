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

-


//====================================================================
//                           MAIN
//====================================================================

int main(int argc, char** argv) {
    // 4. Boilerplate MPI (Ainda não fizemos)
    // MPI_Init, MPI_Comm_rank, etc. 
    return 0;
}