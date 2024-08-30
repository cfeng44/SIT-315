#!/bin/bash

for i in 2 4 6 NUM_CORES 20 40 100 1000; do
    for j in {1..10}; do
        ./partitions-$i
    done
done
