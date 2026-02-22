CC = mpicc
CFLAGS = -fopenmp -Wall -O2

# A regra 'all' é o que roda quando você digita apenas 'make'
all:
	$(CC) $(CFLAGS) *.c -o simulacao

# Uma regra útil para limpar a pasta
clean:
	rm -f simulacao
