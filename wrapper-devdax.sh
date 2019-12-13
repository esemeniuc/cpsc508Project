#!/bin/sh
#$3 is the executable to run
#$4 is the output logging
set -x

if [ "$#" -ne 4 ]; then
    echo "Illegal number of parameters"
fi

VMMALLOC_POOL_DIR=$1 VMMALLOC_POOL_SIZE=$2 LD_PRELOAD=/usr/lib64/libvmmalloc.so.1 $3 >> $4
