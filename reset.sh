#!/bin/bash

#clean allocations
sudo hugeadm --pool-pages-max 2M:0
sudo hugeadm --pool-pages-max 1G:0

#stop service
sudo systemctl stop benchmark
sudo systemctl disable benchmark
sudo rm /etc/systemd/system/benchmark.service

#clean temp files
sudo rm state_setup state_dram4K state_dram2M state_dram1G state_pmem output.txt
rm -f apex-map/Apex.c apex-map/input

find . -type d -name perf_results -exec rm -rf {} \; #delete all folders named 'perf_results'
find . -type d -name fio_results -exec rm -rf {} \; #delete all folders named 'fio_results'
find . -type d -name redis_results -exec rm -rf {} \; #delete all folders named 'redis_results'

#clean prototype hacks
git checkout apex-map
git checkout simple_tests