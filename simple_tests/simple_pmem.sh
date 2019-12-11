#!/bin/bash

gcc memory_stress_pmem.c
sudo perf stat -B -d -d -d ./a.out 19
rm a.out