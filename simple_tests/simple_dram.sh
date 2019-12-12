#!/bin/bash

gcc memory_stress_dram.c
perf stat -B -ddd -e dtlb_load_misses.miss_causes_a_walk ./a.out 19
rm a.out