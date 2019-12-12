#!/bin/sh
perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk fio dram_rand_RW_1G.fio