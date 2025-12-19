#!/bin/bash

# ==============================================================================
# SCRIPT DE AUTOMATIZAÇÃO - TAREFAS A, B, C e D
# Este script compila o projeto e executa as baterias de teste, gerando CSVs.
# ==============================================================================

# Abortar se houver erro
set -e

# Definição de Cores para logs
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}>>> [1/4] Compilando o projeto...${NC}"
make clean
make

# ==============================================================================
# TAREFA A: Fibonacci (Scheduling)
# ==============================================================================
echo -e "${GREEN}>>> [2/4] Executando Tarefa A...${NC}"
FILE_A="resultados_tarefaA.csv"
echo "N,K,Threads,Schedule,Chunk,Tempo" > $FILE_A

# Parâmetros (Ajuste aqui conforme necessário)
Ns=(100000 500000 1000000)
Ks=(20 24 28)
THREADS=(1 2 4 8 16)
SCHEDS=(0 1 2)         # 0=static, 1=dynamic, 2=guided
CHUNKS=(1 4 16 64)     # Chunk sizes

# Mapeamento para nome do Schedule (apenas para log ou CSV se quisesse string)
get_sched_name() {
    case $1 in
        0) echo "static" ;;
        1) echo "dynamic" ;;
        2) echo "guided" ;;
    esac
}

# Loop de Testes
for N in "${Ns[@]}"; do
    for K in "${Ks[@]}"; do
        for T in "${THREADS[@]}"; do
            for S in "${SCHEDS[@]}"; do
                
                # Regra: Chunk 0 só faz sentido para static (auto) ou se quiser testar default
                # Para simplificar, vamos rodar chunks definidos para todos
                
                # Se for static (0) e chunk for diferente de 0, ok.
                # Vamos simplificar e rodar todos CHUNKS para todos SCHEDS
                
                for C in "${CHUNKS[@]}"; do
                    # Configura Threads
                    export OMP_NUM_THREADS=$T
                    
                    # Log visual
                    S_NAME=$(get_sched_name $S)
                    echo "  -> Executando: N=$N K=$K T=$T S=$S_NAME($C)"
                    
                    # Executa e captura apenas o tempo (stdout)
                    # ./tarefaA_omp <N> <K> <sched> <chunk>
                    TEMPO=$(./tarefaA_omp $N $K $S $C)
                    
                    # Salva no CSV: N,K,Threads,ScheduleName,Chunk,Tempo
                    echo "$N,$K,$T,$S_NAME,$C,$TEMPO" >> $FILE_A
                done
                
                # Adiciona caso especial static sem chunk (chunk=0 ou padrão) se desejar
                # Mas o loop acima já cobre bem.
            done
        done
    done
done

# ==============================================================================
# TAREFA B: Histograma (Sincronização)
# ==============================================================================
echo -e "${GREEN}>>> [3/4] Executando Tarefa B...${NC}"
FILE_B="resultados_tarefaB.csv"
echo "N,B,Threads,Variante,Tempo" > $FILE_B

Ns_B=(10000000 50000000 1000000)
Bs=(32 256 4096)
THREADS_B=(1 2 4 8 16)
VARIANTES=(1 2 3) # 1=Critical, 2=Atomic, 3=Reduction

get_var_name() {
    case $1 in
        1) echo "Critical" ;;
        2) echo "Atomic" ;;
        3) echo "Aggregation" ;;
    esac
}

for N in "${Ns_B[@]}"; do
    for B in "${Bs[@]}"; do
        for T in "${THREADS_B[@]}"; do
            for V in "${VARIANTES[@]}"; do
                
                # LÓGICA DE PULO (Skip): Critical é muito lento com N grande e T>1
                if [ "$V" -eq 1 ] && [ "$T" -gt 1 ] && [ "$N" -gt 5000000 ]; then
                    echo "  -> [PULADO] Critical T=$T N=$N (Lento demais)"
                    continue
                fi

                export OMP_NUM_THREADS=$T
                V_NAME=$(get_var_name $V)
                
                echo "  -> Executando: N=$N B=$B T=$T Var=$V_NAME"
                
                # ./tarefaB_omp <N> <B> <Variante>
                TEMPO=$(./tarefaB_omp $N $B $V)
                
                echo "$N,$B,$T,$V_NAME,$TEMPO" >> $FILE_B
            done
        done
    done
done

# ==============================================================================
# TAREFA C: SAXPY (Vetorização SIMD)
# ==============================================================================
echo -e "${GREEN}>>> [4/4] Executando Tarefa C...${NC}"
FILE_C="resultados_tarefaC.csv"
echo "N,Threads,Variante,Tempo" > $FILE_C

Ns_C=(10000000 50000000 1000000) 
THREADS_C=(1 2 4 8 16)
# Variantes C: 1=Seq (Executavel separado), 2=SIMD, 3=Parallel SIMD

for N in "${Ns_C[@]}"; do
    
    # 1. Versão Sequencial Base (Executável separado)
    echo "  -> Executando: N=$N (Sequencial Base)"
    TEMPO_SEQ=$(./tarefaC_seq $N)
    echo "$N,1,Base,$TEMPO_SEQ" >> $FILE_C
    
    # 2. Versão OMP (SIMD Puro - V2)
    # SIMD puro usa apenas 1 thread na teoria (instrução vetorial em 1 core), 
    # mas o programa aceita <variante> 2.
    echo "  -> Executando: N=$N (SIMD V2)"
    TEMPO_SIMD=$(./tarefaC_omp $N 2)
    echo "$N,1,SIMD_V2,$TEMPO_SIMD" >> $FILE_C
    
    # 3. Versão OMP (Parallel SIMD - V3)
    # Aqui variamos threads
    for T in "${THREADS_C[@]}"; do
        export OMP_NUM_THREADS=$T
        echo "  -> Executando: N=$N T=$T (Parallel SIMD V3)"
        TEMPO_PAR=$(./tarefaC_omp $N 3)
        echo "$N,$T,Parallel_SIMD_V3,$TEMPO_PAR" >> $FILE_C
    done

done

# ==============================================================================
# TAREFA D: Overhead de Região Paralela (Fork/Join)
# ==============================================================================
echo -e "${GREEN}>>> [5/5] Executando Tarefa D...${NC}"
FILE_D_RAW="resultados_tarefaD_raw.csv"
FILE_D="resultados_tarefaD.csv"
echo "N,K,B,THREADS,SCHEDULE,T_NAIVE,T_CRIT,T_ATOM,T_LOCAL,T_SIMD" > $FILE_D_RAW
# Parâmetros:
# N pequeno (100k) evidencia o overhead.
# N grande (10M) dilui o overhead no tempo de cálculo.
# Loop de compilação e execução
for N in 100000 500000 1000000; do
  for K in 20 24 28; do
    for B in 32 256 4096; do

      # CORREÇÃO 1: Usar o alvo correto (tarefaD_omp) e forçar recompilação (-B)
      # Usamos -B para garantir que ele recompile com os novos defines de N, K e B
      make -B tarefaD_omp N=$N K=$K B=$B > /dev/null

      for T in 1 2 4 8 16; do
        export OMP_NUM_THREADS=$T

        for S in "static,1" "static,64" "dynamic,1"; do
          export OMP_SCHEDULE=$S

          for R in {1..5}; do
            # CORREÇÃO 2: Executar o binário correto gerado pelo Makefile
            ./tarefaD_omp >> $FILE_D_RAW
          done
        done

      done
    done
  done
done

# Cabeçalho do arquivo final agregado
echo "N,K,B,THREADS,SCHEDULE,NAIVE_MEAN,NAIVE_STD,CRIT_MEAN,CRIT_STD,ATOM_MEAN,ATOM_STD,LOCAL_MEAN,LOCAL_STD,SIMD_MEAN,SIMD_STD" > $FILE_D

# AWK para calcular médias e desvios das 5 variantes
awk -F, '
{
  key = $1","$2","$3","$4","$5
  n[key]++
  
  tn[key]+=$6;  tn2[key]+=$6*$6   # Naive
  tc[key]+=$7;  tc2[key]+=$7*$7   # Critical
  ta[key]+=$8;  ta2[key]+=$8*$8   # Atomic
  tl[key]+=$9;  tl2[key]+=$9*$9   # Local
  ts[key]+=$10; ts2[key]+=$10*$10 # Simd
}
END {
  for (k in n) {
    mn=tn[k]/n[k]; sn=sqrt(tn2[k]/n[k]-mn*mn)
    mc=tc[k]/n[k]; sc=sqrt(tc2[k]/n[k]-mc*mc)
    ma=ta[k]/n[k]; sa=sqrt(ta2[k]/n[k]-ma*ma)
    ml=tl[k]/n[k]; sl=sqrt(tl2[k]/n[k]-ml*ml)
    ms=ts[k]/n[k]; ss=sqrt(ts2[k]/n[k]-ms*ms)
    
    print k","mn","sn","mc","sc","ma","sa","ml","sl","ms","ss
  }
}' $FILE_D_RAW >> $FILE_D

echo -e "${GREEN}>>> Todos os testes concluídos! CSVs gerados.${NC}"