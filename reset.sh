#!/bin/bash

sudo hugeadm --pool-pages-max 2M:0
sudo hugeadm --pool-pages-max 1G:0

find . -type d -name perf_results -exec rm -rf {} \; #delete all folders named 'perf_results'