#!/bin/bash

TEST_NAMES=("2mm_time"
    "3mm_time"
    "atax_time"
    "cholesky_time"
    "durbin_time"
    "lu_time"
    "trisolv_time"
    "gramschmidt_time")

for ((i = 0; i < ${#TEST_NAMES[@]}; ++i)); do
  sudo perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk ../wrapper-devdax.sh /dev/dax0.0 $((28*1024*1024*1024)) bin/${TEST_NAMES[i]} perf_results/${TEST_NAMES[i]}_perf.log
done
