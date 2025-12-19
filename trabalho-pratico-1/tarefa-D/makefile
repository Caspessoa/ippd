CC=gcc
CFLAGS=-O3 -Wall
OMPFLAGS=-fopenmp

N ?= 1000000
K ?= 20
B ?= 256

BIN_SEQ=bin/seq
BIN_OMP=bin/omp

all: seq omp

seq:
	mkdir -p bin
	$(CC) $(CFLAGS) -DN=$(N) -DK=$(K) -DB=$(B) \
	src/seq/region_seq.c -o $(BIN_SEQ)

omp:
	mkdir -p bin
	$(CC) $(CFLAGS) $(OMPFLAGS) \
	-DN=$(N) -DK=$(K) -DB=$(B) \
	src/omp/region_omp.c -o $(BIN_OMP)

run:
	bash run.sh

clean:
	rm -rf bin results.csv
