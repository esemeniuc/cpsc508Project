#!/bin/bash

gcc memory_stress_pmem.c
mkdir -p perf_results
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/simple_pmem.log ./a.out 19
rm a.out