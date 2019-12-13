#!/bin/bash
set -x

#apex
(cd apex-map || exit
./apex_dram.sh) #using subshell

#fio
(cd fio || exit
./dram.sh)

#polybench
(cd polybench || exit
./compile_polybench.sh
./polybench_dram.sh
rm -rf bin)

#redis
(cd redis || exit
./redis_dram.sh)

#simple_tests
(cd simple_tests || exit
./simple_dram.sh)