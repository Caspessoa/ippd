#!/bin/bash

echo "Iniciando a compilação..."

# O asterisco (*.c) pega todos os arquivos .c da pasta atual.
# -fopenmp: Ativa o suporte ao OpenMP
# -Wall: Mostra todos os avisos (warnings) do código, ótimo para achar erros
# -O2: Ativa otimizações de performance do compilador (importante para simulações)
# -o simulacao: Define o nome do executável final

mpicc -fopenmp -Wall -O2 *.c -o simulacao

# Verifica se a compilação deu certo
if [ $? -eq 0 ]; then
    echo "Compilação concluída com sucesso! Executável 'simulacao' gerado."
    echo "Para rodar use, por exemplo: mpirun -np 4 ./simulacao"
else
    echo "Erro na compilação."
fi
