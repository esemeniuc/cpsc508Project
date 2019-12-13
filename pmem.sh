#!/bin/bash
set -x

#redis
(cd redis || exit
./redis_pmem.sh)

#apex
(cd apex-map || exit
./apex_pmem.sh) #using subshell

#fio
(cd fio || exit
./pmem.sh)

#polybench
(cd polybench || exit
./compile_polybench.sh
./polybench_pmem.sh
rm -rf bin)

#simple_tests
(cd simple_tests || exit
./simple_pmem.sh)