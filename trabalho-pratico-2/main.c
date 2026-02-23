#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h> 
#include <stddef.h>
#include <unistd.h> // Necessário para a função usleep()
// --- Definições de Tipos e Estruturas ---

typedef enum {
    ALDEIA,
    PESCA,
    COLETA,
    ROCADO,
    INTERDITA
} TipoCelula;

typedef enum {
    SECA,
    CHEIA
} Estacao;

typedef struct {
    TipoCelula tipo;     
    double recurso;      
    bool acessivel;      
} Celula;

typedef struct {
    int x, y;            // Posições locais ao processo
    int gx, gy;          // Posições globais (facilita migração)
    double energia;      
} Agente;

#define MAX_CUSTO_CARGA 1000000

#define T_TOTAL 100    // Ciclos totais 
#define S_SAZONAL 10   // Ciclos por estação 
#define MAX_AG_THREAD 1000 // Tamanho máximo do buffer por thread

// --- Estrutura para Buffers de Thread ---
typedef struct {
    Agente agentes[MAX_AG_THREAD];
    int count;
} ThreadBuffer;

// Definições de cores para o terminal
#define RESET  "\x1B[0m"
#define RED    "\x1B[31m"
#define GRN    "\x1B[32m"
#define BLU    "\x1B[34m"
#define YEL    "\x1B[33m"
#define MAG    "\x1B[35m"
#define CYAN   "\x1B[36m"

void visualizar_subgrid(int rank, int W_local, int H_local, int offsetY, Celula* grid, Agente* agentes, int n_agentes) {
    // Pequena pausa para não atropelar a impressão de outros processos
    MPI_Barrier(MPI_COMM_WORLD); 
    
    printf("\n--- Processo MPI Rank %d (Linhas Globais: %d a %d) ---\n", rank, offsetY, offsetY + H_local - 1);

    for (int j = 0; j < H_local; j++) {
        for (int i = 0; i < W_local; i++) {
            int idx = j * W_local + i;
            bool tem_agente = false;

            // Verifica se há algum agente nesta célula local
            for (int k = 0; k < n_agentes; k++) {
                if (agentes[k].x == i && agentes[k].y == j) {
                    tem_agente = true;
                    break;
                }
            }

            if (tem_agente) {
                printf(RED " @ " RESET); // Agente representado por @
            } else {
                switch (grid[idx].tipo) {
                    case ALDEIA:    printf(MAG " H " RESET); break;
                    case PESCA:     printf(BLU " ~ " RESET); break;
                    case COLETA:    printf(GRN " f " RESET); break;
                    case ROCADO:    printf(YEL " # " RESET); break;
                    case INTERDITA: printf(" X "); break;
                }
            }
        }
        printf("\n");
    }
    fflush(stdout);
}
// --- Funções de Suporte (Lógica de Negócio) ---

// Determina o tipo de terreno com base na posição global (Consistência)
TipoCelula f_tipo(int gx, int gy) {
    // Exemplo matemático simples para garantir que o mapa seja o mesmo em todos os processos
    int val = (gx * 31 + gy * 7) % 5;
    return (TipoCelula)abs(val);
}

// Define o recurso inicial com base no tipo
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

void executar_carga(double recurso) {
    long iteracoes = (long)(recurso * 1000); 
    if (iteracoes > MAX_CUSTO_CARGA) iteracoes = MAX_CUSTO_CARGA;

    volatile double dummy = 0.0;
    for (long c = 0; c < iteracoes; c++) {
        dummy += (c * 0.0001); 
    }
}

// --- Main (Orquestração MPI) ---

int main(int argc, char** argv) {
    // 1. Inicialização do Ambiente 
    MPI_Init(&argc, &argv);

    // Definição do Tipo Derivado MPI para a struct Agente
    MPI_Datatype mpi_agente_type;
    int blocklengths[3] = {2, 2, 1}; // {x, y}, {gx, gy}, {energia}
    MPI_Aint displacements[3];
    displacements[0] = offsetof(Agente, x);
    displacements[1] = offsetof(Agente, gx);
    displacements[2] = offsetof(Agente, energia);
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_DOUBLE};
    
    MPI_Type_create_struct(3, blocklengths, displacements, types, &mpi_agente_type);
    MPI_Type_commit(&mpi_agente_type);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 

    // Dimensões Globais 
    int W_global = 20;
    int H_global = 20;
    int n_agentes_total = 40; // Exemplo 

    // Particionamento (Decomposição de Domínio)
    int W_local = W_global;
    int H_local = H_global / size; 

    // Offsets para situar o subgrid no mundo global 
    int offsetX = 0;
    int offsetY = rank * H_local;

    // Alocação do Grid Local
    Celula* grid_local = (Celula*)malloc(W_local * H_local * sizeof(Celula));

    // Inicialização do Grid (Garantindo continuidade global) 
    for (int j = 0; j < H_local; j++) {
        for (int i = 0; i < W_local; i++) {
            int gx = offsetX + i; 
            int gy = offsetY + j; 

            int idx = j * W_local + i;
            grid_local[idx].tipo = f_tipo(gx, gy);
            grid_local[idx].recurso = f_recurso(grid_local[idx].tipo); 
            grid_local[idx].acessivel = true; 
        }
    }

    // Inicialização de Agentes Locais
    // Criar agentes apenas se a posição global deles pertencer ao meu subgrid 
    // 4. Inicialização de Agentes Locais
    int n_agentes_locais = n_agentes_total / size; 
    int capacidade_agentes = n_agentes_locais + 1000; // Margem de sobra para evitar reallocs excessivos
    Agente* lista_agentes = (Agente*)malloc(capacidade_agentes * sizeof(Agente));

    srand(42 + rank); // Semente diferente por processo para agentes não nascerem no mesmo lugar
    for (int i = 0; i < n_agentes_locais; i++) {
        lista_agentes[i].gx = rand() % W_global;
        lista_agentes[i].gy = offsetY + (rand() % H_local); // Garante que nasce no meu território
        lista_agentes[i].x = lista_agentes[i].gx - offsetX;
        lista_agentes[i].y = lista_agentes[i].gy - offsetY;
        lista_agentes[i].energia = 100.0;
    }

    printf("[Processo %d] Grid inicializado. OffsetY: %d, Agentes: %d\n", rank, offsetY, n_agentes_locais);

    // --- Fim da Sprint 2 ---

    Estacao estacao_atual = SECA;
    // Sincroniza todos os processos antes de iniciar o cronômetro
    MPI_Barrier(MPI_COMM_WORLD);
    double tempo_inicio = 0.0;
    if (rank == 0) {
        tempo_inicio = MPI_Wtime();
    }

    // ==========================================================================================================
    // INÍCIO DO LOOP DA SIMULAÇÃO (t = 0 até T_TOTAL)
    // ==========================================================================================================
    for (int t = 0; t < T_TOTAL; t++) {

        // --- 5.1) Atualizar Estação ---
        if (rank == 0) {
            if (t > 0 && t % S_SAZONAL == 0) {
                estacao_atual = (estacao_atual == SECA) ? CHEIA : SECA;
            }
        }
        MPI_Bcast(&estacao_atual, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // --- 5.2) Troca de Halo (Bordas do Grid) ---
        Celula halo_superior[W_global];
        Celula halo_inferior[W_global];

        int vizinho_cima = (rank == 0) ? MPI_PROC_NULL : rank - 1;
        int vizinho_baixo = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

        MPI_Sendrecv(&grid_local[0], W_local * sizeof(Celula), MPI_BYTE, vizinho_cima, 0,
                     halo_superior, W_local * sizeof(Celula), MPI_BYTE, vizinho_cima, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&grid_local[(H_local - 1) * W_local], W_local * sizeof(Celula), MPI_BYTE, vizinho_baixo, 1,
                     halo_inferior, W_local * sizeof(Celula), MPI_BYTE, vizinho_baixo, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // --- 5.3) Processar Agentes (OpenMP) --- 
        ThreadBuffer* buffers_locais = (ThreadBuffer*)malloc(omp_get_max_threads() * sizeof(ThreadBuffer));

        int max_buffer = (n_agentes_locais > 0) ? n_agentes_locais : 1;
        Agente* buffer_envio_cima = (Agente*)malloc(max_buffer * sizeof(Agente));
        Agente* buffer_envio_baixo = (Agente*)malloc(max_buffer * sizeof(Agente));
        int contagem_cima = 0, contagem_baixo = 0;

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            buffers_locais[tid].count = 0;

            #pragma omp for 
            for (int i = 0; i < n_agentes_locais; i++) {
                Agente *a = &lista_agentes[i];
                int idx = a->y * W_local + a->x;
                
                // 1. Carga sintética
                executar_carga(grid_local[idx].recurso);

                // 2. Lógica simplificada de movimento (Random Walk com Paredes Globais)
                int dx = (rand() % 3) - 1; 
                int dy = (rand() % 3) - 1;
                
                int novo_x = a->x + dx;
                int novo_y = a->y + dy;

                // Bloqueia a saída pelas laterais Leste/Oeste (Eixo X)
                if (novo_x < 0) novo_x = 0;
                if (novo_x >= W_local) novo_x = W_local - 1;

                // Bloqueia a saída pelos extremos Norte/Sul Globais (Eixo Y)
                if (novo_y < 0 && vizinho_cima == MPI_PROC_NULL) novo_y = 0;
                if (novo_y >= H_local && vizinho_baixo == MPI_PROC_NULL) novo_y = H_local - 1;

                // Aplica a atualização de movimento de forma segura
                a->gx += (novo_x - a->x);
                a->gy += (novo_y - a->y);
                a->x = novo_x;
                a->y = novo_y;

                // Verifica se o agente continua no grid local ou se vai migrar
                if (a->y >= 0 && a->y < H_local) {
                    if (buffers_locais[tid].count < MAX_AG_THREAD) {
                        buffers_locais[tid].agentes[buffers_locais[tid].count] = *a; 
                        buffers_locais[tid].count++;
                    }
                } else {
                    // Se saiu do limite Y, vai para outro processo MPI
                    #pragma omp critical
                    {
                        if (a->y < 0 && vizinho_cima != MPI_PROC_NULL) {
                            buffer_envio_cima[contagem_cima++] = *a;
                        } else if (a->y >= H_local && vizinho_baixo != MPI_PROC_NULL) {
                            buffer_envio_baixo[contagem_baixo++] = *a;
                        }
                    }
                }
            }
        }

        // Consolidação dos buffers locais na lista principal de agentes 
        n_agentes_locais = 0;
        for (int i = 0; i < omp_get_max_threads(); i++) {
            for (int j = 0; j < buffers_locais[i].count; j++) {
                lista_agentes[n_agentes_locais++] = buffers_locais[i].agentes[j];
            }
        }
        free(buffers_locais);

        // --- 5.4) Migração de Agentes (MPI) --- 
        int num_recv_cima = 0, num_recv_baixo = 0;

        // Troca de tamanhos
        MPI_Sendrecv(&contagem_cima, 1, MPI_INT, vizinho_cima, 2,
                     &num_recv_cima, 1, MPI_INT, vizinho_cima, 3,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&contagem_baixo, 1, MPI_INT, vizinho_baixo, 3,
                     &num_recv_baixo, 1, MPI_INT, vizinho_baixo, 2,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Troca de dados reais (Requer o mpi_agente_type criado anteriormente)
        Agente* recv_cima = malloc((num_recv_cima > 0 ? num_recv_cima : 1) * sizeof(Agente));
        Agente* recv_baixo = malloc((num_recv_baixo > 0 ? num_recv_baixo : 1) * sizeof(Agente));

        MPI_Sendrecv(buffer_envio_cima, contagem_cima, mpi_agente_type, vizinho_cima, 4,
                     recv_cima, num_recv_cima, mpi_agente_type, vizinho_cima, 5,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(buffer_envio_baixo, contagem_baixo, mpi_agente_type, vizinho_baixo, 5,
                     recv_baixo, num_recv_baixo, mpi_agente_type, vizinho_baixo, 4,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int novo_total = n_agentes_locais + num_recv_cima + num_recv_baixo;
        if (novo_total > capacidade_agentes) {
            capacidade_agentes = novo_total + 1000; // Cresce com folga
            lista_agentes = (Agente*)realloc(lista_agentes, capacidade_agentes * sizeof(Agente));
            if (lista_agentes == NULL) {
                printf("[Rank %d] Erro fatal de memoria no realloc!\n", rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        // Adicionar agentes recebidos à lista local e ajustar Y local
        for (int i = 0; i < num_recv_cima; i++) {
            recv_cima[i].y = H_local - 1; 
            lista_agentes[n_agentes_locais++] = recv_cima[i];
        }
        for (int i = 0; i < num_recv_baixo; i++) {
            recv_baixo[i].y = 0; 
            lista_agentes[n_agentes_locais++] = recv_baixo[i];
        }
        
        // Liberar a memória dinâmica criada neste ciclo estritamente uma vez
        free(recv_cima); 
        free(recv_baixo);
        free(buffer_envio_cima);
        free(buffer_envio_baixo);

        // --- 5.5) Atualizar Grid Local (OpenMP) ---
        #pragma omp parallel for collapse(2) 
        for (int j = 0; j < H_local; j++) {
            for (int i = 0; i < W_local; i++) {
                int idx = j * W_local + i;
                double taxa = (estacao_atual == SECA) ? 1.5 : 3.0;
                grid_local[idx].recurso += taxa; 
            }
        }

        // --- 5.6) Métricas globais (MPI) ---
        int total_agentes_local = n_agentes_locais;
        double energia_total_local = 0.0;
        double recurso_total_local = 0.0;

        #pragma omp parallel for reduction(+:energia_total_local)
        for (int i = 0; i < n_agentes_locais; i++) {
            energia_total_local += lista_agentes[i].energia;
        }

        #pragma omp parallel for collapse(2) reduction(+:recurso_total_local)
        for (int j = 0; j < H_local; j++) {
            for (int i = 0; i < W_local; i++) {
                int idx = j * W_local + i;
                recurso_total_local += grid_local[idx].recurso;
            }
        }

        int total_agentes_global = 0;
        double energia_total_global = 0.0;
        double recurso_total_global = 0.0;

        MPI_Allreduce(&total_agentes_local, &total_agentes_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
        MPI_Allreduce(&energia_total_local, &energia_total_global, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
        MPI_Allreduce(&recurso_total_local, &recurso_total_global, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 

        MPI_Barrier(MPI_COMM_WORLD); 

        // Impressão das Estatísticas
        if (rank == 0 && t % 10 == 0) { 
            printf("\n=== ESTATÍSTICAS GLOBAIS - CICLO %d ===\n", t);
            printf("Estação atual: %s\n", (estacao_atual == SECA ? "SECA" : "CHEIA"));
            printf("População Total (Agentes): %d\n", total_agentes_global);
            printf("Energia Acumulada: %.2f\n", energia_total_global);
            printf("Recursos Globais do Território: %.2f\n", recurso_total_global);
            printf("=======================================\n");
        }
//--------------------------------------------------------------------------------------------

        // --- 5.7) Visualização (Animação no Terminal) ---
        
        // 1. Sincroniza e limpa o terminal
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            system("clear"); // Limpa a tela para o próximo "frame"
        }
        MPI_Barrier(MPI_COMM_WORLD); // Garante que a tela foi limpa antes de começar a imprimir

        // 2. Cada processo imprime sua fatia de tela
        visualizar_subgrid(rank, W_local, H_local, offsetY, grid_local, lista_agentes, n_agentes_locais);
        
        // 3. Pausa a execução para o olho humano conseguir ver o movimento
        // 300000 microssegundos = 0.3 segundos por ciclo
        usleep(400000); 

        
        //-----------------Fim visualização----------------

    } // FIM DO LAÇO FOR (t)

    // ==========================================================================================================
    // FIM DA SIMULAÇÃO - MEDIÇÃO DE TEMPO E FINALIZAÇÃO
    // ==========================================================================================================
    
    // Sincroniza todos os processos no final para a medição ser justa
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        double tempo_fim = MPI_Wtime();
        printf("\nTempo total de execucao: %f segundos\n", tempo_fim - tempo_inicio);
    } 

    // Limpeza final de tipos MPI criados manualmente
   // MPI_Type_free(&mpi_agente_type);
    
    // Finalização básica para teste
    free(grid_local);
    free(lista_agentes);
    // Limpeza final de tipos MPI criados manualmente
    MPI_Type_free(&mpi_agente_type);
    MPI_Finalize(); 
    return 0;
}