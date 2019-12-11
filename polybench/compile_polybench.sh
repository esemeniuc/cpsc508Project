#!/bin/bash

POLYBENCH_TESTS_DIR=("linear-algebra/kernels/2mm"
    "linear-algebra/kernels/3mm"
    "linear-algebra/kernels/atax"
    "linear-algebra/solvers/cholesky"
		"linear-algebra/solvers/durbin"
    "linear-algebra/solvers/lu"
		"linear-algebra/solvers/trisolv"
		"linear-algebra/solvers/gramschmidt")

POLYBENCH_TESTS_SOURCE=("linear-algebra/kernels/2mm/2mm.c"
		"linear-algebra/kernels/3mm/3mm.c"
		"linear-algebra/kernels/atax/atax.c"
		"linear-algebra/solvers/cholesky/cholesky.c"
    "linear-algebra/solvers/durbin/durbin.c"
    "linear-algebra/solvers/lu/lu.c"
    "linear-algebra/solvers/trisolv/trisolv.c"
    "linear-algebra/solvers/gramschmidt/gramschmidt.c")

TEST_NAMES=("2mm_time" 
    "3mm_time"
    "atax_time"
    "cholesky_time"
    "durbin_time"
    "lu_time"
    "trisolv_time"
    "gramschmidt_time")

SRC_DIR=polybench-c-4.2.1-beta
mkdir -p bin
mkdir -p perf_results

for ((i = 0; i < ${#POLYBENCH_TESTS_DIR[@]}; ++i)); do
  gcc -O3 -I ${SRC_DIR}/utilities \
          -I ${SRC_DIR}/${POLYBENCH_TESTS_DIR[i]} \
          ${SRC_DIR}/utilities/polybench.c \
          ${SRC_DIR}/${POLYBENCH_TESTS_SOURCE[i]} \
          -DPOLYBENCH_TIME -o bin/${TEST_NAMES[i]} -lm
done