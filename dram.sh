#!/bin/bash

#apex

#fio
cd fio || exit #script expects to be in directory
./run_dram_rand_RW_1G.sh
./run_dram_seq_R_1G.sh