#include "logger.h"
#include <stdio.h>

void iniciar_log(const char *nome_arquivo) {
  // Abre no modo "w" (write) para apagar o log da execução anterior e começar
  // limpo
  FILE *f = fopen(nome_arquivo, "w");
  if (f != NULL) {
    fprintf(f, "=== HISTÓRICO DA SIMULAÇÃO (MPI + OpenMP) ===\n");
    fprintf(
        f,
        "Ciclo | Estação | População Total | Energia Total | Recurso Total\n");
    fprintf(
        f, "---------------------------------------------------------------\n");
    fclose(f);
  } else {
    printf("Erro ao criar o arquivo de log!\n");
  }
}

void registrar_log_ciclo(const char *nome_arquivo, int ciclo, Estacao estacao,
                         int populacao, double energia, double recursos) {
  // Abre no modo "a" (append) para adicionar a nova linha no final do arquivo
  FILE *f = fopen(nome_arquivo, "a");
  if (f != NULL) {
    fprintf(f, "%5d | %7s | %15d | %13.2f | %13.2f\n", ciclo,
            (estacao == SECA ? "SECA" : "CHEIA"), populacao, energia, recursos);
    fclose(f);
  }
}
