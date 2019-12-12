#!/bin/bash

T_LOC_ARR=(1.0 0.0001)
mkdir -p perf_results

for i in {1..16}; do
    S_LOC=$((2**i))
    for T_LOC in ${T_LOC_ARR[@]}; do
        sed "s/UGRAD_SPATIAL_LOCALITY/${S_LOC}/" input_template | sed "s/UGRAD_TEMPORAL_LOCALITY/${T_LOC}/" > input
        gcc gen.pub.c -lm -w > /dev/null
        ./a.out #run script generator
        gcc Apex.c -lm -w > /dev/null
        sudo perf stat -ddd -x '|' -e task-clock -e dtlb_load_misses.miss_causes_a_walk --append -o perf_results/dram_${TLB_SIZE}.log ${HUGECTL_CMD} ./a.out > /dev/null
    done
done

rm input a.out Apex.c