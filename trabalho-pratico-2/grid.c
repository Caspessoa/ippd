#include <stdlib.h>
#include "grid.h"

TipoCelula f_tipo(int gx, int gy) {
    int val = (gx * 31 + gy * 7) % 5;
    return (TipoCelula)abs(val);
}

double f_recurso(TipoCelula tipo) {
    switch (tipo) {
        case ALDEIA: return 100.0;
        case PESCA:  return 50.0;
        case COLETA: return 30.0;
        case ROCADO: return 80.0;
        case INTERDITA: return 0.0;
        default: return 10.0;
    }
}
