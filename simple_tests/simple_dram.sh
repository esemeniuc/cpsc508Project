#!/bin/bash

gcc memory_stress_dram.c
perf stat -B -d -d -d ./a.out 19
rm a.out