#!/bin/bash

sudo hugeadm --pool-pages-max 2M:0
sudo hugeadm --pool-pages-max 1G:0
sudo systemctl disable benchmark
sudo rm /etc/systemd/system/benchmark.service
sudo rm state_setup state_dram4K state_dram2M state_dram1G

find . -type d -name perf_results -exec rm -rf {} \; #delete all folders named 'perf_results'