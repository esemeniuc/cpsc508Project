#!/bin/bash

gcc memory_stress_dram.c
mkdir -p perf_results
perf stat -ddd -x '|' -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/simple_dram.log ./a.out 19
rm a.out