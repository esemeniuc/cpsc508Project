#!/bin/bash
#set -x

#apex
cd apex-map/apex-map || exit
./apex_dram.sh
cd ..

#fio
cd fio || exit #script expects to be in directory
./dram.sh
cd ..

#polybench
cd polybench || exit
./compile_polybench.sh
./polybench_dram.sh
rm -rf bin
cd ..

#simple_tests
cd simple_tests || exit
./simple_dram.sh
cd ..
