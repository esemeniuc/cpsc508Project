#!/bin/bash
set -x
mkdir -p perf_results
mkdir -p redis_results
VMMALLOC_POOL_DIR=/dev/dax0.0 VMMALLOC_POOL_SIZE=30064771072 LD_PRELOAD=/usr/local/lib64/vmem_debug/libvmmalloc.so.1.0.0 redis-server &
sleep 10 #let server boot up
redis-benchmark -n 8000 -r 100 -t GET,SET -d 512000 --csv >> redis_results/pmem_${TLB_SIZE}.log
sudo killall redis-server
rm -f dump.rdb
