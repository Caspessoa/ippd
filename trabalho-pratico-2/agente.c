#include <stdlib.h>
#include <omp.h>
#include "agente.h"

// Fator multiplicativo para a carga. Ajuste para não travar o PC!
#define FATOR_CUSTO 1000 
#define LIMITE_CUSTO 50000 // Limite para evitar cargas excessivas

/* * A carga sintética representa o custo computacional proporcional
 * ao recurso da célula[cite: 45, 46].
 */
void executar_carga(double recurso) {
    long iteracoes = (long)(recurso * FATOR_CUSTO);

    // Limita o custo máximo conforme exigido no documento [cite: 49]
    if (iteracoes > LIMITE_CUSTO) {
        iteracoes = LIMITE_CUSTO;
    }

    // ATENÇÃO: A palavra 'volatile' é crucial aqui. 
    // Como usamos a flag de otimização -O2 na compilação, o compilador 
    // é inteligente e removeria um "for" que não faz nada útil. 
    // O 'volatile' obriga a CPU a realmente fazer as contas, 
    // simulando o processamento[cite: 47, 48, 170].
    volatile double trabalho_falso = 0.0;
    for (long c = 0; c < iteracoes; c++) {
        trabalho_falso += 1.0; 
    }
}

/*
 * Consome o recurso de uma célula[cite: 37].
 * Como o OpenMP vai processar vários agentes ao mesmo tempo,
 * dois agentes podem tentar comer o recurso da MESMA célula na mesma hora.
 */
void consumir_recurso(double *recurso_celula, double quantidade) {
    // A diretiva 'atomic' garante que apenas uma thread por vez 
    // subtraia o valor desta variável específica na memória.
    #pragma omp atomic
    *recurso_celula -= quantidade;

    // Evita que o recurso fique negativo
    if (*recurso_celula < 0.0) {
        #pragma omp atomic write
        *recurso_celula = 0.0;
    }
}

/*
 * Lógica provisória de movimento (Random Walk).
 * O agente decide se mover para uma célula vizinha ou ficar parado[cite: 36].
 */
void decidir_movimento(Agente *a, int max_x, int max_y) {
    // Gera um número entre -1, 0 e 1 para x e y
    int dx = (rand() % 3) - 1;
    int dy = (rand() % 3) - 1;

    // Atualiza a posição, garantindo que não saia dos limites do subgrid
    // (A lógica de sair do subgrid para ir para outro processo MPI virá depois no main)
    int novo_x = a->x + dx;
    int novo_y = a->y + dy;

    if (novo_x >= 0 && novo_x < max_x) a->x = novo_x;
    if (novo_y >= 0 && novo_y < max_y) a->y = novo_y;

    // Diminui um pouco de energia a cada turno
    a->energia -= 1.0; 
}
