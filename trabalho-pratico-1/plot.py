import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# ==============================================================================
# CONFIGURAÇÕES GERAIS
# ==============================================================================
OUTPUT_DIR = "images"
sns.set_theme(style="whitegrid")
plt.rcParams.update({'figure.max_open_warning': 0})

def setup_ambiente():
    """Cria a pasta de images se não existir."""
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        print(f"Diretório '{OUTPUT_DIR}' criado.")

def calcular_speedup(df, group_cols, base_condition_col='Threads', base_condition_val=1):
    """
    Função auxiliar para calcular Speedup em um DataFrame.
    Speedup = Tempo_Sequencial / Tempo_Paralelo
    """
    # Cria uma cópia para não alterar o original durante iteração se for slice
    df = df.copy()
    
    # Encontra o tempo base (T=1) para cada grupo
    # Ex: Para cada N e K, qual o tempo quando Threads=1?
    
    # Agrupa pelas colunas que definem o cenário (ex: N, K)
    # Mas precisamos aplicar o speedup linha a linha.
    
    # Estratégia: Criar um dicionário de tempos base
    tempos_base = {}
    
    # Identifica configurações únicas
    configs = df[group_cols].drop_duplicates().values
    
    for conf in configs:
        # Filtra o subconjunto
        mask = pd.Series([True] * len(df), index=df.index)
        for col, val in zip(group_cols, conf):
            mask &= (df[col] == val)
            
        subset = df[mask]
        
        # Tenta achar o tempo base (menor tempo com 1 thread ou base sequencial)
        # No caso do Bash script, temos múltiplas entradas T=1 (scheds diferentes).
        # Pegamos a média ou o mínimo dos tempos com T=1 como referência.
        try:
            tempo_ref = subset[subset[base_condition_col] == base_condition_val]['Tempo'].min()
        except:
            tempo_ref = None
            
        if pd.isna(tempo_ref):
             # Fallback: pega o máximo tempo do grupo (pior caso assumido como seq)
             tempo_ref = subset['Tempo'].max()

        # Aplica o speedup nas linhas correspondentes
        df.loc[mask, 'Speedup'] = tempo_ref / df.loc[mask, 'Tempo']
        
    return df

# ==============================================================================
# TAREFA A: Fibonacci (Scheduling)
# ==============================================================================
def plot_tarefa_A():
    arquivo = 'resultados_tarefaA.csv'
    if not os.path.exists(arquivo):
        print(f"Aviso: {arquivo} não encontrado. Pulando Tarefa A.")
        return

    print("Gerando gráficos da Tarefa A...")
    df = pd.read_csv(arquivo)
    
    # Cria coluna combinada para legenda
    df['Estrategia'] = df['Schedule'] + " (" + df['Chunk'].astype(str) + ")"
    
    # Calcula Speedup
    df = calcular_speedup(df, group_cols=['N', 'K'])

    configs = df[['N', 'K']].drop_duplicates().values
    for n, k in configs:
        subset = df[(df['N'] == n) & (df['K'] == k)]
        
        # 1. Tempo x Threads
        plt.figure(figsize=(10, 6))
        sns.lineplot(data=subset, x='Threads', y='Tempo', hue='Estrategia', marker='o')
        plt.title(f'Tarefa A: Tempo de Execução - N={n}, K={k}')
        plt.ylabel('Tempo (s)')
        plt.xlabel('Threads')
        plt.xticks(sorted(df['Threads'].unique()))
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.tight_layout()
        plt.savefig(f"{OUTPUT_DIR}/A_Tempo_N{n}_K{k}.png")
        plt.close()

        # 2. Speedup x Threads
        plt.figure(figsize=(10, 6))
        sns.lineplot(data=subset, x='Threads', y='Speedup', hue='Estrategia', marker='o')
        plt.title(f'Tarefa A: Speedup - N={n}, K={k}')
        plt.axhline(1, color='red', linestyle='--', label='Sequencial (1x)')
        plt.axhline(2, color='green', linestyle=':', label='Ideal Dual-Core (2x)')
        plt.ylabel('Speedup')
        plt.xlabel('Threads')
        plt.xticks(sorted(df['Threads'].unique()))
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.tight_layout()
        plt.savefig(f"{OUTPUT_DIR}/A_Speedup_N{n}_K{k}.png")
        plt.close()

# ==============================================================================
# TAREFA B: Histograma
# ==============================================================================
def plot_tarefa_B():
    arquivo = 'resultados_tarefaB.csv'
    if not os.path.exists(arquivo):
        print(f"Aviso: {arquivo} não encontrado. Pulando Tarefa B.")
        return

    print("Gerando gráficos da Tarefa B...")
    df = pd.read_csv(arquivo)
    
    configs = df[['N', 'B']].drop_duplicates().values
    for n, b in configs:
        subset = df[(df['N'] == n) & (df['B'] == b)]
        
        plt.figure(figsize=(10, 6))
        sns.lineplot(data=subset, x='Threads', y='Tempo', hue='Variante', style='Variante', markers=True, linewidth=2)
        
        tipo_contencao = "Alta Contenção" if b < 100 else "Baixa Contenção"
        plt.title(f'Tarefa B: {tipo_contencao} - N={n}, Buckets={b}')
        plt.ylabel('Tempo (s)')
        plt.xlabel('Threads')
        plt.xticks(sorted(df['Threads'].unique()))
        plt.grid(True, which='both', linestyle='--')
        plt.tight_layout()
        plt.savefig(f"{OUTPUT_DIR}/B_Tempo_N{n}_B{b}.png")
        plt.close()

# ==============================================================================
# TAREFA C: SAXPY (SIMD)
# ==============================================================================
def plot_tarefa_C():
    arquivo = 'resultados_tarefaC.csv'
    if not os.path.exists(arquivo):
        print(f"Aviso: {arquivo} não encontrado. Pulando Tarefa C.")
        return

    print("Gerando gráficos da Tarefa C...")
    df = pd.read_csv(arquivo)
    
    # Calcula Speedup comparado à base "Base" (T=1 do sequencial)
    # Aqui a lógica é levemente diferente pois temos uma variante chamada "Base"
    configs = df['N'].unique()
    
    for n in configs:
        subset = df[df['N'] == n].copy()
        
        # Pega tempo base
        try:
            tempo_base = subset[subset['Variante'] == 'Base']['Tempo'].values[0]
            subset['Speedup'] = tempo_base / subset['Tempo']
        except:
            subset['Speedup'] = 0 # Erro se não achar base
            
        # 1. Bar Plot Comparativo (Base vs SIMD vs Parallel Max Threads)
        # Filtra para pegar o melhor caso de Parallel (max threads)
        max_threads = subset['Threads'].max()
        
        # Seleciona: Base, SIMD_V2, e Parallel_SIMD_V3 (apenas com max threads ou todas para ver escalada)
        # Vamos simplificar mostrando todas as Variantes/Threads relevantes
        
        plt.figure(figsize=(12, 6))
        sns.barplot(data=subset, x='Variante', y='Speedup', hue='Threads', palette='viridis')
        plt.title(f'Tarefa C: Impacto SIMD - N={n}')
        plt.ylabel('Speedup (vs Base)')
        plt.axhline(1, color='red', linestyle='--')
        plt.tight_layout()
        plt.savefig(f"{OUTPUT_DIR}/C_Comparacao_N{n}.png")
        plt.close()
        
        # 2. Linha de Escalabilidade (Apenas V3)
        v3 = subset[subset['Variante'] == 'Parallel_SIMD_V3']
        if not v3.empty:
            plt.figure(figsize=(10, 6))
            sns.lineplot(data=v3, x='Threads', y='Speedup', marker='o')
            
            # Adiciona linha do SIMD puro como referência
            v2_val = subset[subset['Variante'] == 'SIMD_V2']['Speedup'].max()
            if pd.notna(v2_val):
                plt.axhline(v2_val, color='orange', linestyle='--', label=f'SIMD Single ({v2_val:.2f}x)')
                plt.legend()
                
            plt.title(f'Tarefa C: Escalabilidade Parallel SIMD - N={n}')
            plt.ylabel('Speedup Total')
            plt.xticks(sorted(v3['Threads'].unique()))
            plt.tight_layout()
            plt.savefig(f"{OUTPUT_DIR}/C_Escalabilidade_N{n}.png")
            plt.close()



# ==============================================================================
# TAREFA D: Overhead de Região Paralela
# ==============================================================================
def plot_tarefa_D():
    # 1. Carregar CSV
    try:
        df = pd.read_csv("resultados_tarefaD.csv")
    except FileNotFoundError:
        print("Erro: Arquivo resultados_tarefaD.csv não encontrado.")
        sys.exit(1)

    # 2. Limpeza Robusta de Dados
    # Força colunas numéricas a serem números. Erros (como cabeçalhos repetidos) viram NaN
    cols_numeric = ["N", "K", "B", "THREADS", "NAIVE_MEAN", "CRIT_MEAN", "ATOM_MEAN", "LOCAL_MEAN", "SIMD_MEAN"]
    for col in cols_numeric:
        df[col] = pd.to_numeric(df[col], errors='coerce')

    # Remove linhas que contêm NaN (isso remove os cabeçalhos repetidos no meio do arquivo)
    df = df.dropna()

    # Converte N, K, B, THREADS de volta para inteiro (após limpeza)
    df["N"] = df["N"].astype(int)
    df["K"] = df["K"].astype(int)
    df["B"] = df["B"].astype(int)
    df["THREADS"] = df["THREADS"].astype(int)

    # 3. Filtros
    target_n = 1000000
    target_k = 20
    target_b = 256

    base = df[
        (df["N"] == target_n) &
        (df["K"] == target_k) &
        (df["B"] == target_b)
    ]

    if base.empty:
        print(f"Erro: Dados vazios após filtro N={target_n}, K={target_k}, B={target_b}.")
        print("Dados disponíveis (amostra):")
        print(df[['N','K','B']].drop_duplicates().head())
        sys.exit(1)

    plt.rcParams.update({'font.size': 12})

    # =========================================================
    # Gráfico 1: Tarefa D - Overhead (Naive vs Local)
    # =========================================================
    plt.figure(figsize=(10, 6))

    scheds = base["SCHEDULE"].unique()
    print(f"Schedules encontrados: {scheds}")

    for sched in scheds:
        # Filtra static-64 (agora com traço, devido à correção no C)
        if "static" in sched and "64" in sched: 
            sub = base[base["SCHEDULE"] == sched].sort_values("THREADS")
            
            plt.plot(sub["THREADS"], sub["NAIVE_MEAN"], marker="x", linestyle="--", label=f"Naive (2 regions)")
            plt.plot(sub["THREADS"], sub["LOCAL_MEAN"], marker="o", label=f"Optimized (1 region)")
            plt.title(f"Overhead de Região Paralela ({sched})")

    plt.xlabel("Threads")
    plt.ylabel("Tempo (s)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/tarefaD_overhead.png", dpi=150)
    print("Gerado: tarefaD_overhead.png")

    # =========================================================
    # Gráfico 2: Comparação Geral (Escala Log)
    # =========================================================
    plt.figure(figsize=(10, 6))
    # Pega o primeiro schedule que contém 'static' como representativo
    rep_sched = next((s for s in scheds if "static" in s), scheds[0])
    sub = base[base["SCHEDULE"] == rep_sched].sort_values("THREADS")

    plt.errorbar(sub["THREADS"], sub["CRIT_MEAN"], yerr=sub.get("CRIT_STD", 0), fmt="-^", label="Critical")
    plt.errorbar(sub["THREADS"], sub["ATOM_MEAN"], yerr=sub.get("ATOM_STD", 0), fmt="-s", label="Atomic")
    plt.errorbar(sub["THREADS"], sub["LOCAL_MEAN"], yerr=sub.get("LOCAL_STD", 0), fmt="-o", label="Local Aggregation")

    plt.xlabel("Threads")
    plt.ylabel("Tempo (s)")
    plt.title(f"Comparação de Sincronização ({rep_sched})")
    plt.legend()
    plt.grid(True)
    plt.yscale('log')
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/comparacao_sincronizacao_D.png", dpi=150)
    print("Gerado: comparacao_sincronizacao_D.png")

    # =========================================================
    # Gráfico 3: SIMD
    # =========================================================
    plt.figure(figsize=(10, 6))

    w = 0.35
    plt.bar(sub["THREADS"] - w/2, sub["LOCAL_MEAN"], width=w, label="No SIMD")
    plt.bar(sub["THREADS"] + w/2, sub["SIMD_MEAN"], width=w, label="With SIMD")

    plt.xlabel("Threads")
    plt.ylabel("Tempo (s)")
    plt.title(f"Ganho de Vetorização SIMD ({rep_sched})")
    plt.xticks(sub["THREADS"])
    plt.legend()
    plt.grid(True, axis='y')
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/ganho_simd_D.png", dpi=150)
    print("Gerado: ganho_simd_D.png")


# ==============================================================================
# EXECUÇÃO PRINCIPAL
# ==============================================================================
if __name__ == "__main__":
    setup_ambiente()
    plot_tarefa_A()
    plot_tarefa_B()
    plot_tarefa_C()
    plot_tarefa_D()

    print(f"\nConcluído! Verifique a pasta '{OUTPUT_DIR}/'.")