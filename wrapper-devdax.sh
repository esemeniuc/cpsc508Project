#!/bin/sh

if [ "$#" -ne 4 ]; then
    echo "Illegal number of parameters"
fi

touch $3
touch $4

echo VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=/home/cs508/cpsc508Project/vmem/src/debug/libvmmalloc.so $3 >> $4

VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=/home/cs508/cpsc508Project/vmem/src/debug/libvmmalloc.so $3 >> $4
