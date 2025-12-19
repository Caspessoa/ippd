# INTRODUÇÃO AO PENSAMENTO PARALELO E DISTRIBUÍDO

# RESULTADOS

## Comparação: critical vs atomic vs agregação local

### Metodologia

-   Cada ponto: média de 5 execuções
-   Avaliados múltiplos N, K, B, threads e schedules
-   Medição de tempo total dos kernels

### Observações

-   `critical` apresenta o pior desempenho devido à serialização explícita
-   `atomic` reduz o overhead, mas ainda sofre contenção para muitos threads
-   Agregação local é consistentemente a melhor abordagem
-   `schedule(static)` apresentou menor variância
-   `dynamic` introduziu overhead sem benefício para carga uniforme

### Conclusão

Sempre que possível, deve-se preferir agregação local.  
`atomic` é aceitável apenas para atualizações simples.  
`critical` deve ser evitado em regiões de alto custo.
