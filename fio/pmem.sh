#!/bin/sh
set -x
mkdir -p perf_results
mkdir -p fio_results
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/seq_R_pmem_${TLB_SIZE}.log fio pmem_seq_R_1G.fio >> fio_results/seq_R_pmem_${TLB_SIZE}_fio_results.log
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/rand_RW_pmem_${TLB_SIZE}.log fio pmem_rand_RW_1G.fio >> fio_results/rand_RW_pmem_${TLB_SIZE}_fio_results.log
