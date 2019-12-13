#!/bin/bash
set -x

#apex
(cd apex-map/apex-map || exit
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