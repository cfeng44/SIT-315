#!/bin/zsh

# For each executable in current directory, run 100 times and display the
# average at the end.
for cpp_exe in *.out; do
    total=0
    num_tests=100

    echo "--- ${cpp_exe} ---\n"

    for _ in {1..$num_tests}; do
        result=$(./$cpp_exe)
        total=$(echo "${total} + ${result}" | bc)

        echo -n "`./$cpp_exe` "
    done

    average=$(echo "scale=2; $total / $num_tests" | bc)
    echo "\n\nAverage: $average \xC2\xB5s\n"
done