#!/bin/zsh


num_tests=100

echo "--- seq ---\n"

total=0
for _ in {1..$num_tests}; do
    result=$(./seq.out)
    total=$(echo "${total} + ${result}" | bc)

    echo -n "$result "
done

average=$(echo "scale=2; $total / $num_tests" | bc)
echo "\n\nAverage: $average \xC2\xB5s\n"

echo "--- par ---\n"

total=0
for _ in {1..$num_tests}; do
    result=$(./par.out)
    total=$(echo "${total} + ${result}" | bc)

    echo -n "$result "
done

average=$(echo "scale=2; $total / $num_tests" | bc)
echo "\n\nAverage: $average \xC2\xB5s\n"

echo "--- omp ---\n"

total=0
for _ in {1..$num_tests}; do
    result=$(./omp.out)
    total=$(echo "${total} + ${result}" | bc)

    echo -n "$result "
done

average=$(echo "scale=2; $total / $num_tests" | bc)
echo "\n\nAverage: $average \xC2\xB5s\n"


echo "--- mpi ---\n"

total=0
for _ in {1..$num_tests}; do
    result=$(mpirun -np 10 ./mpi.out)
    total=$(echo "${total} + ${result}" | bc)

    echo -n "$result "
done

average=$(echo "scale=2; $total / $num_tests" | bc)
echo "\n\nAverage: $average \xC2\xB5s\n"