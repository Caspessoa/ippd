# Variáveis de Configuração
CC = gcc
CFLAGS = -Wall -std=gnu99
OMP_FLAGS = -fopenmp
OPT_FLAGS = -O3
SEQ_SRC_DIR = src/seq
OMP_SRC_DIR = src/omp
BIN_DIR = .

# Nomes dos executáveis
SEQ_A_EXEC = $(BIN_DIR)/tarefaA_seq
OMP_A_EXEC = $(BIN_DIR)/tarefaA_omp
OMP_B_SRC = src/omp/tarefaB_omp.c
OMP_B_EXEC = ./tarefaB_omp
SEQ_C_EXEC = $(BIN_DIR)/tarefaC_seq
OMP_C_EXEC = $(BIN_DIR)/tarefaC_omp

.PHONY: all clean run

all: tarefaA_seq tarefaA_omp tarefaB_omp tarefaC_seq tarefaC_omp

# --------------------------------------------------------------------------
# Alvos para a Tarefa A
# --------------------------------------------------------------------------

# Compilação da versão sequencial (base)
tarefaA_seq: $(SEQ_SRC_DIR)/tarefaA_seq.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(SEQ_SRC_DIR)/tarefaA_seq.c -o $(SEQ_A_EXEC) $(CFLAGS) $(OPT_FLAGS) -lrt 
	@echo "Compilado: $(SEQ_A_EXEC)"

# Compilação da versão paralela (OpenMP)
tarefaA_omp: $(OMP_SRC_DIR)/tarefaA_omp.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(OMP_SRC_DIR)/tarefaA_omp.c -o $(OMP_A_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_A_EXEC)"

#Flags especiais para SAXPY
# Base: Desliga vetorização automática para servir de comparação justa
tarefaC_seq: $(SEQ_SRC_DIR)/tarefaC_seq.c
	$(CC) $(SEQ_SRC_DIR)/tarefaC_seq.c -o $(SEQ_C_EXEC) $(CFLAGS) $(OPT_FLAGS) -fno-tree-vectorize -lrt

# OMP: Liga tudo (OpenMP + Otimizações)
tarefaC_omp: $(OMP_SRC_DIR)/tarefaC_omp.c
	$(CC) $(OMP_SRC_DIR)/tarefaC_omp.c -o $(OMP_C_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)

# --- Regra de Compilação da Tarefa B ---
tarefaB_omp: $(OMP_B_SRC)
	$(CC) $(OMP_B_SRC) -o $(OMP_B_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_B_EXEC)"


#
# --------------------------------------------------------------------------
# Limpeza e Execução (Provisório)
# --------------------------------------------------------------------------

clean:
	rm -f *_seq *_omp *.csv *.png

# Atalho para rodar (O script Python faz isso melhor, mas deixamos o make pronto)
run: all
	python run_tarefaA.py