#!/bin/sh
#$3 is the executable to run
#$4 is the output logging
set -x

if [ "$#" -ne 4 ]; then
    echo "Illegal number of parameters"
fi
#VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=/home/cs508/cpsc508Project/vmem/src/debug/libvmmalloc.so $3 >> $4

VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=/usr/local/lib64/vmem_debug/libvmmalloc.so.1.0.0 $3 >> $4
