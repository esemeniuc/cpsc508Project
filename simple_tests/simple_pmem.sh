#!/bin/bash

gcc memory_stress_pmem.c
mkdir -p perf_results
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/pmem_${TLB_SIZE}.log ../wrapper-devdax.sh /dev/dax0.0 $((28*1024*1024*1024)) "./a.out 19" ../output.txt
rm a.out