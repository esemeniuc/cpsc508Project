#!/bin/sh
perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk fio dram_seq_R_1G.fio