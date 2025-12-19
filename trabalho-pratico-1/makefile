# ==========================================================================
# CONFIGURAÇÕES DE COMPILAÇÃO
# ==========================================================================
CC = gcc
CFLAGS = -Wall -std=gnu99
OMP_FLAGS = -fopenmp
OPT_FLAGS = -O3
LDFLAGS = -lrt -lm

# Diretórios
SEQ_SRC_DIR = src/seq
OMP_SRC_DIR = src/omp
BIN_DIR = .

# --- DEFINIÇÃO DOS EXECUTÁVEIS ---
# Tarefa A (Fibonacci)
SEQ_A_EXEC = $(BIN_DIR)/tarefaA_seq
OMP_A_EXEC = $(BIN_DIR)/tarefaA_omp

# Tarefa B (Histograma)
OMP_B_SRC  = src/omp/tarefaB_omp.c
OMP_B_EXEC = ./tarefaB_omp

# Tarefa C (SAXPY - SIMD)
SEQ_C_EXEC = $(BIN_DIR)/tarefaC_seq
OMP_C_EXEC = $(BIN_DIR)/tarefaC_omp

# Tarefa D (Overhead Fork/Join)
OMP_D_SRC  = src/omp/tarefaD.c
OMP_D_EXEC = ./tarefaD

# Phony targets (alvos que não são arquivos)
.PHONY: all clean run

# ==========================================================================
# ALVO PRINCIPAL (DEFAULT) - Apenas Compila
# ==========================================================================
all: tarefaA_seq tarefaA_omp tarefaB_omp tarefaC_seq tarefaC_omp tarefaD
	@echo "--- Todos os binários foram compilados com sucesso. ---"
	@echo "Para rodar os testes e gerar gráficos, digite: make run"

# ==========================================================================
# ALVO DE AUTOMATIZAÇÃO (Compila -> Roda Scripts -> Plota)
# ==========================================================================
run: all
	@echo ""
	@echo "========================================"
	@echo ">>> 1. Rodando Testes da TAREFA A..."
	@echo "========================================"
	python3 run_tarefaA.py
	
	@echo ""
	@echo "========================================"
	@echo ">>> 2. Rodando Testes da TAREFA B..."
	@echo "========================================"
	python3 run_tarefaB.py
	
	@echo ""
	@echo "========================================"
	@echo ">>> 3. Rodando Testes da TAREFA C..."
	@echo "========================================"
	python3 run_tarefaC.py
	
	@echo ""
	@echo "========================================"
	@echo ">>> 4. Rodando Testes da TAREFA D..."
	@echo "========================================"
	python3 run_tarefaD.py

	@echo ""
	@echo "========================================"
	@echo ">>> 5. Gerando TODOS os Gráficos..."
	@echo "========================================"
	python3 plot.py
	@echo ""
	@echo ">>> PROCESSO COMPLETO! Verifique a pasta 'images/'."

# ==========================================================================
# REGRAS DE COMPILAÇÃO
# ==========================================================================

# --- Tarefa A ---
tarefaA_seq: $(SEQ_SRC_DIR)/tarefaA_seq.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(SEQ_SRC_DIR)/tarefaA_seq.c -o $(SEQ_A_EXEC) $(CFLAGS) $(OPT_FLAGS) $(LDFLAGS)
	@echo "Compilado: $(SEQ_A_EXEC)"

tarefaA_omp: $(OMP_SRC_DIR)/tarefaA_omp.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(OMP_SRC_DIR)/tarefaA_omp.c -o $(OMP_A_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS) $(LDFLAGS)
	@echo "Compilado: $(OMP_A_EXEC)"

# --- Tarefa B ---
tarefaB_omp: $(OMP_B_SRC)
	$(CC) $(OMP_B_SRC) -o $(OMP_B_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_B_EXEC)"

# --- Tarefa C ---
tarefaC_seq: $(SEQ_SRC_DIR)/tarefaC_seq.c
	# Flag -fno-tree-vectorize para impedir otimização automática e servir de base
	$(CC) $(SEQ_SRC_DIR)/tarefaC_seq.c -o $(SEQ_C_EXEC) $(CFLAGS) $(OPT_FLAGS) -fno-tree-vectorize $(LDFLAGS)
	@echo "Compilado: $(SEQ_C_EXEC)"

tarefaC_omp: $(OMP_SRC_DIR)/tarefaC_omp.c
	$(CC) $(OMP_SRC_DIR)/tarefaC_omp.c -o $(OMP_C_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_C_EXEC)"

# --- Tarefa D ---
tarefaD: $(OMP_D_SRC)
	$(CC) $(OMP_D_SRC) -o $(OMP_D_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_D_EXEC)"

# ==========================================================================
# LIMPEZA
# ==========================================================================
clean:
	rm -f *_seq *_omp tarefaD *.csv
	rm -rf images/
	@echo "Limpeza concluída."