import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# ==============================================================================
# CONFIGURAÇÕES GERAIS
# ==============================================================================
OUTPUT_DIR = "imagens"
sns.set_theme(style="whitegrid")
plt.rcParams.update({'figure.max_open_warning': 0})

def setup_ambiente():
    """Cria a pasta de imagens se não existir."""
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
    arquivo = 'resultados_tarefaD.csv'
    if not os.path.exists(arquivo):
        print(f"Aviso: {arquivo} não encontrado. Pulando Tarefa D.")
        return

    print("Gerando gráficos da Tarefa D...")
    df = pd.read_csv(arquivo)
    
    # Garante ordenação para o gráfico de linha não ficar bagunçado
    df = df.sort_values(by=['N', 'Threads', 'Variante'])

    configs = df['N'].unique()
    for n in configs:
        subset = df[df['N'] == n]
        
        # --- GRÁFICO 1: TEMPO DE EXECUÇÃO (Comparativo) ---
        plt.figure(figsize=(10, 6))
        sns.lineplot(
            data=subset, 
            x='Threads', 
            y='Tempo', 
            hue='Variante', 
            style='Variante', 
            markers=True, 
            dashes=False
        )
        plt.title(f'Tarefa D: Impacto do Overhead (Fork/Join) - N={n}')
        plt.ylabel('Tempo (segundos)')
        plt.xlabel('Threads')
        plt.xticks(sorted(df['Threads'].unique()))
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.tight_layout()
        plt.savefig(f"{OUTPUT_DIR}/D_Tempo_N{n}.png")
        plt.close()
        
        # --- GRÁFICO 2: GANHO RELATIVO (Smart / Naive) ---
        # Calculamos: Tempo_Naive / Tempo_Smart
        # Se for > 1.0, significa que a Smart é mais rápida (menos overhead)
        try:
            # Pivotar a tabela para ter as variantes lado a lado
            pivoted = subset.pivot(index='Threads', columns='Variante', values='Tempo')
            
            # Identifica as colunas (nomes podem variar dependendo do script usado)
            cols = pivoted.columns
            # Procura por palavras chave
            col_naive = next((c for c in cols if 'Naive' in c or 'V1' in c), None)
            col_smart = next((c for c in cols if 'Smart' in c or 'V2' in c), None)

            if col_naive and col_smart:
                pivoted['Ratio'] = pivoted[col_naive] / pivoted[col_smart]
                
                plt.figure(figsize=(10, 6))
                sns.lineplot(data=pivoted, x=pivoted.index, y='Ratio', marker='s', color='purple')
                
                plt.title(f'Tarefa D: Ganho da Versão Otimizada vs Ingênua - N={n}')
                plt.ylabel('Quantas vezes mais rápido (Speedup Relativo)')
                plt.xlabel('Threads')
                plt.axhline(1, color='red', linestyle='--', label='Igualdade (1x)')
                plt.legend()
                plt.grid(True)
                plt.tight_layout()
                plt.savefig(f"{OUTPUT_DIR}/D_Ratio_N{n}.png")
                plt.close()
        except Exception as e:
            print(f"  Aviso: Não foi possível gerar gráfico de Ratio para N={n}. Erro: {e}")


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