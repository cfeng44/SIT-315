#!/bin/zsh

# Compiles each version.
g++-14 -std=c++11 /Users/codey/od/315/Module2/pt1/seq/Sequential.cpp -o seqx
g++-14 -std=c++11 /Users/codey/od/315/Module2/pt1/par/Parallel.cpp -o parx -lpthread
g++-14 -std=c++11 /Users/codey/od/315/Module2/pt1/omp/OMP.cpp -o ompx -fopenmp

# Run sequential version.
for _ in {1..5}; do
    ./seqx | grep time
done

# Run pthreads version.
for _ in {1..5}; do
    ./parx | grep time
done

# Run OMP version.
for _ in {1..5}; do
    ./ompx | grep time
done
