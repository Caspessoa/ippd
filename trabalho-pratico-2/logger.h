#ifndef LOGGER_H
#define LOGGER_H

#include "grid.h"

// Assinaturas das funções
void iniciar_log(const char* nome_arquivo);
void registrar_log_ciclo(const char* nome_arquivo, int ciclo, Estacao estacao, int populacao, double energia, double recursos);

#endif
