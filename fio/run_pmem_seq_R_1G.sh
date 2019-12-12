#!/bin/sh
perf stat -B -ddd -e dtlb_load_misses.miss_causes_a_walk fio pmem_seq_R_1G.fio
