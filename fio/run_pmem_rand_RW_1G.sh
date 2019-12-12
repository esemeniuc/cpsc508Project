#!/bin/sh
perf stat -B -ddd -e dtlb_load_misses.miss_causes_a_walk fio pmem_rand_RW_1G.fio
