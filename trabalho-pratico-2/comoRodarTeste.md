A primeira vez que rodar tem que dar permissão de execução pq por padrão
arquivos de texto criados não têm permissão para rodar como programas. 

Execute o comando: chmod +x benchmark.sh

Depois para executar o script: ./benchmark.sh



PARA RODAR O TESTE É MELHOR COMENTAR O TRECHO QUE IMPRIME A MATRIZ NA TELA
PQ VAI AUMENTAR O CUSTO/TEMPO

O TRECHO É DA LINHA 399 ATÉ 413:

//--------------------------------------------------------------------------------------------

        // --- 5.7) Visualização (Animação no Terminal) ---
        
        // 1. Sincroniza e limpa o terminal
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            system("clear"); // Limpa a tela para o próximo "frame"
        }
        MPI_Barrier(MPI_COMM_WORLD); // Garante que a tela foi limpa antes de começar a imprimir

        // 2. Cada processo imprime sua fatia de tela
        visualizar_subgrid(rank, W_local, H_local, offsetY, grid_local, lista_agentes, n_agentes_locais);
        
        // 3. Pausa a execução para o olho humano conseguir ver o movimento
        // 300000 microssegundos = 0.3 segundos por ciclo
        usleep(400000); 

        
        //-----------------Fim visualização----------------