// Estrutura do Agente sugerida pelo professor
typedef struct {
    int x, y;
    double energia; // ou "necessidade" [cite: 32]
} Agente;

// Estrutura da Célula do Grid [cite: 20]
typedef struct {
    int tipo;           // aldeia, pesca, coleta, roçado ou interditada [cite: 21]
    double recurso;     // capacidade local para sustentar agentes [cite: 22]
    int acessivel;      // depende da estação do ano [cite: 23]
} Celula;
