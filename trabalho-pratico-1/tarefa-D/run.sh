#!/bin/bash

RAW=results_raw.csv
OUT=results.csv

echo "N,K,B,THREADS,SCHEDULE,T_CRIT,T_ATOM,T_LOCAL" > $RAW

for N in 100000 500000 1000000; do
  for K in 20 24 28; do
    for B in 32 256 4096; do

      make omp N=$N K=$K B=$B > /dev/null

      for T in 1 2 4 8 16; do
        export OMP_NUM_THREADS=$T

        for S in "static,1" "static,64" "dynamic,1"; do
          export OMP_SCHEDULE=$S

          for R in {1..5}; do
            ./bin/omp >> $RAW
          done
        done

      done
    done
  done
done

echo "N,K,B,THREADS,SCHEDULE,CRIT_MEAN,CRIT_STD,ATOM_MEAN,ATOM_STD,LOCAL_MEAN,LOCAL_STD" > $OUT

awk -F, '
{
  key = $1","$2","$3","$4","$5
  n[key]++
  tc[key]+=$6; tc2[key]+=$6*$6
  ta[key]+=$7; ta2[key]+=$7*$7
  tl[key]+=$8; tl2[key]+=$8*$8
}
END {
  for (k in n) {
    mc=tc[k]/n[k]; sc=sqrt(tc2[k]/n[k]-mc*mc)
    ma=ta[k]/n[k]; sa=sqrt(ta2[k]/n[k]-ma*ma)
    ml=tl[k]/n[k]; sl=sqrt(tl2[k]/n[k]-ml*ml)
    print k","mc","sc","ma","sa","ml","sl
  }
}' $RAW >> $OUT
