#!/bin/bash

mkdir -p perf_results
mkdir -p redis_results
(sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/pmem_${TLB_SIZE}.log ../wrapper-devdax.sh /dev/dax0.0 $((28*1024*1024*1024)) redis-server ../output.txt) &
sleep 10 #let server boot up
redis-benchmark -n 8000 -r 100 -t GET,SET -d 512000 --csv >> redis_results/pmem_${TLB_SIZE}.log
sudo killall redis-server
rm -f dump.rdb
