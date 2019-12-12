#!/bin/bash

sudo hugeadm --pool-pages-max 2M:0
sudo hugeadm --pool-pages-max 1G:0
sudo systemctl disable benchmark
sudo rm /etc/systemd/system/benchmark.service

find . -type d -name perf_results -exec rm -rf {} \; #delete all folders named 'perf_results'