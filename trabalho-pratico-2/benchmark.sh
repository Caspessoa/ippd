#!/bin/bash

# Nome do executável e arquivo de saída
EXEC="./simulacao"
OUTPUT_FILE="resultados_benchmark.txt"

# Limpa o arquivo de saída anterior
echo "=== Benchmark: Simulação Híbrida MPI + OpenMP ===" > $OUTPUT_FILE
echo "Data do teste: $(date)" >> $OUTPUT_FILE
echo "-------------------------------------------------" >> $OUTPUT_FILE

# Compila o código (garante que estamos testando a versão mais recente)
echo "Compilando o código..."
mpicc -fopenmp main.c -o simulacao -lm
if [ $? -ne 0 ]; then
    echo "Erro na compilação. Abortando testes."
    exit 1
fi
echo "Compilação concluída com sucesso!"
echo "Iniciando bateria de testes..."

# Arrays de configuração para o teste de escalabilidade
PROCESSOS_MPI=(1 2 4)       # Testar com 1, 2 e 4 processos MPI
THREADS_OPENMP=(1 2 4 8)    # Testar com 1, 2, 4 e 8 threads por processo

for p in "${PROCESSOS_MPI[@]}"; do
    for t in "${THREADS_OPENMP[@]}"; do
        
        echo "Testando -> MPI: $p processos | OpenMP: $t threads..."
        
        # Configura a variável de ambiente do OpenMP
        export OMP_NUM_THREADS=$t
        
        # Grava o cabeçalho no arquivo de saída
        echo "" >> $OUTPUT_FILE
        echo "[Configuração] Processos MPI: $p | Threads OpenMP: $t" >> $OUTPUT_FILE
        
        # Executa o programa e redireciona a saída para o arquivo
        # O comando 'grep' filtra para capturar apenas a linha do tempo e ignorar o resto
        mpirun -np $p $EXEC | grep "Tempo total" >> $OUTPUT_FILE
        
    done
done

echo "-------------------------------------------------" >> $OUTPUT_FILE
echo "Bateria de testes concluída. Resultados salvos em $OUTPUT_FILE."
cat $OUTPUT_FILE