#!/bin/sh
#$3 is the executable to run
#$4 is the output logging

if [ "$#" -ne 4 ]; then
    echo "Illegal number of parameters"
fi

echo VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=libvmmalloc.so.1 $3 >> $4

VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=libvmmalloc.so.1 $3 >> $4
