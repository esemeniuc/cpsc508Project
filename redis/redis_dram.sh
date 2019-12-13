#!/bin/bash

mkdir -p perf_results
mkdir -p redis_results
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/dram_${TLB_SIZE}.log ${HUGECTL_CMD} redis-server &
redis-benchmark -n 10000 -r 100 -t GET,SET -d 512000 --csv >> redis_results/dram_${TLB_SIZE}.log
sudo killall redis-server
