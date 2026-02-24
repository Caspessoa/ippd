#include "agente.h"

void executar_carga(double recurso) {
    long iteracoes = (long)(recurso * 1000); 
    if (iteracoes > MAX_CUSTO_CARGA) iteracoes = MAX_CUSTO_CARGA;

    // volatile evita que a otimização -O2 do compilador remova o laço
    volatile double dummy = 0.0;
    for (long c = 0; c < iteracoes; c++) {
        dummy += (c * 0.0001); 
    }
}
