# ==========================================================================
# CONFIGURAÇÕES E VARIÁVEIS
# ==========================================================================
CC = gcc
CFLAGS = -Wall -std=gnu99
C_FLAGS = -O3 -Wall
OMP_FLAGS = -fopenmp
OPT_FLAGS = -O3
SEQ_SRC_DIR = src/seq
OMP_SRC_DIR = src/omp
BIN_DIR = .

N ?= 1000000
K ?= 20
B ?= 256

# Nomes dos executáveis
SEQ_A_EXEC = $(BIN_DIR)/tarefaA_seq
OMP_A_EXEC = $(BIN_DIR)/tarefaA_omp
OMP_B_SRC  = src/omp/tarefaB_omp.c
OMP_B_EXEC = ./tarefaB_omp
SEQ_C_EXEC = $(BIN_DIR)/tarefaC_seq
OMP_C_EXEC = $(BIN_DIR)/tarefaC_omp
SEQ_D_EXEC = $(BIN_DIR)/tarefaD_seq
OMP_D_EXEC = $(BIN_DIR)/tarefaD_omp

# Phony targets (alvos que não são arquivos)
.PHONY: all clean run plot

# ==========================================================================
# ALVO PADRÃO (Apenas Compila)
# ==========================================================================
all: tarefaA_seq tarefaA_omp tarefaB_omp tarefaC_seq tarefaC_omp tarefaD_seq tarefaD_omp
	@echo "--- Todos os binários foram compilados com sucesso. ---"
	@echo "Para rodar os testes, digite: make run"

# ==========================================================================
# ALVO DE EXECUÇÃO (Compila -> Roda -> Plota)
# ==========================================================================
run: all
	@echo ""
	@echo ">>> 2. Executando Bateria de Testes (run.sh)..."
	@chmod +x run.sh
	@./run.sh
	@echo ""
	@echo ">>> 3. Gerando Gráficos (plot.py)..."
	@python3 plot.py
	@echo ""
	@echo ">>> PROCESSO COMPLETO! Verifique a pasta 'images/'."

# ==========================================================================
# REGRAS DE COMPILAÇÃO
# ==========================================================================

# --- Tarefa A ---
tarefaA_seq: $(SEQ_SRC_DIR)/tarefaA_seq.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(SEQ_SRC_DIR)/tarefaA_seq.c -o $(SEQ_A_EXEC) $(CFLAGS) $(OPT_FLAGS) -lrt 
	@echo "Compilado: $(SEQ_A_EXEC)"

tarefaA_omp: $(OMP_SRC_DIR)/tarefaA_omp.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(OMP_SRC_DIR)/tarefaA_omp.c -o $(OMP_A_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_A_EXEC)"

# --- Tarefa B ---
tarefaB_omp: $(OMP_B_SRC)
	$(CC) $(OMP_B_SRC) -o $(OMP_B_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_B_EXEC)"

# --- Tarefa C (SAXPY) ---
tarefaC_seq: $(SEQ_SRC_DIR)/tarefaC_seq.c
	$(CC) $(SEQ_SRC_DIR)/tarefaC_seq.c -o $(SEQ_C_EXEC) $(CFLAGS) $(OPT_FLAGS) -fno-tree-vectorize -lrt
	@echo "Compilado: $(SEQ_C_EXEC)"

tarefaC_omp: $(OMP_SRC_DIR)/tarefaC_omp.c
	$(CC) $(OMP_SRC_DIR)/tarefaC_omp.c -o $(OMP_C_EXEC) $(CFLAGS) $(OPT_FLAGS) $(OMP_FLAGS)
	@echo "Compilado: $(OMP_C_EXEC)"

# --- Tarefa D ---
tarefaD_seq: $(SEQ_SRC_DIR)/tarefaD_seq.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(C_FLAGS) $(OMP_FLAGS) -DN=$(N) -DK=$(K) -DB=$(B) $(SEQ_SRC_DIR)/tarefaD_seq.c -o $(SEQ_D_EXEC)
	@echo "Compilado: $(SEQ_D_EXEC)"

tarefaD_omp: $(OMP_SRC_DIR)/tarefaD_omp.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(C_FLAGS) $(OMP_FLAGS) -DN=$(N) -DK=$(K) -DB=$(B) $(OMP_SRC_DIR)/tarefaD_omp.c -o $(OMP_D_EXEC)
	@echo "Compilado: $(OMP_D_EXEC)"

# ==========================================================================
# LIMPEZA
# ==========================================================================
clean:
	rm -f *_seq *_omp *.csv
	rm -rf images/
	@echo "Limpeza concluída."