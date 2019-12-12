#!/bin/sh
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/seq_R_dram_${TLB_SIZE}.log fio dram_seq_R_1G.fio >> seq_R_dram_${TLB_SIZE}_fio_results.log
sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/rand_RW_dram_${TLB_SIZE}.log fio dram_rand_RW_1G.fio >> rand_RW_dram_${TLB_SIZE}_fio_results.log
