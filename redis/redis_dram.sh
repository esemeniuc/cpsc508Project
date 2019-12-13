#!/bin/bash
set -x
mkdir -p perf_results
mkdir -p redis_results
${HUGECTL_CMD} redis-server &
sleep 10 #let server boot up
redis-benchmark -n 8000 -r 100 -t GET,SET -d 512000 --csv >> redis_results/dram_${TLB_SIZE}.log
sudo killall redis-server
rm -f dump.rdb
