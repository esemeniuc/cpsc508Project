#!/bin/sh

POLYBENCH_TESTS_DIR=("linear-algebra/kernels/2mm"
                "linear-algebra/kernels/3mm"
                "linear-algebra/kernels/atax"
	        "linear-algebra/solvers/cholesky"
		"linear-algebra/solvers/durbin"
        	"linear-algebra/solvers/lu"
		"linear-algebra/solvers/trisolv"
		"linear-algebra/solvers/gramschmidt")

POLYBENCH_TESTS_SOURCE=("linear-algebra/kernels/2mm/2mm.c"
		"linear-algebra/kernels/3mm/3mm.c"
		"linear-algebra/kernels/atax/atax.c"
		"linear-algebra/solvers/cholesky/cholesky.c"
                "linear-algebra/solvers/durbin/durbin.c"
                "linear-algebra/solvers/lu/lu.c"
                "linear-algebra/solvers/trisolv/trisolv.c"
                "linear-algebra/solvers/gramschmidt/gramschmidt.c")

TEST_NAMES=("2mm_time" 
	    "3mm_time" 
	    "atax_time" 
	    "cholesky_time" 
	    "durbin_time" 
	    "lu_time" 
	    "trisolv_time" 
	    "gramschmidt_time")
echo > ../polybench_compile.txt
rm -rf ../polybench_compiled_tests 
mkdir ../polybench_compiled_tests

rm -rf ../result_logs
mkdir ../result_logs

rm -rf ../perf_results
mkdir ../perf_results

for ((i = 0; i < ${#TEST_NAMES[@]}; ++i))
do 
  touch ../polybench_compiled_tests/${TEST_NAMES[i]}
done

for ((i = 0; i < ${#POLYBENCH_TESTS_DIR[@]}; ++i))
do 
  echo ${TEST_NAMES[i]} >> ../polybench_compile.txt
  gcc -O3 -I utilities -I ${POLYBENCH_TESTS_DIR[i]} utilities/polybench.c ${POLYBENCH_TESTS_SOURCE[i]} -DPOLYBENCH_TIME -o ${TEST_NAMES[i]} -lm
done

for ((i = 0; i < ${#TEST_NAMES[@]}; ++i))
do
  mv ${TEST_NAMES[i]} ../polybench_compiled_tests/
  touch ../result_logs/${TEST_NAMES[i]}.log
  touch ../perf_results/${TEST_NAMES[i]}_perf.log
done

for ((i = 0; i < ${#TEST_NAMES[@]}; ++i))
do
  echo perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk ../../wrapper-devdax.sh /dev/dax0.0 $((28*1024*1024*1024)) /home/cs508/cpsc508Project/polybench/polybench_compiled_tests/${TEST_NAMES[i]} ../result_logs/${TEST_NAMES[i]}.log
  perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk ../../wrapper-devdax.sh /dev/dax0.0 $((28*1024*1024*1024)) /home/cs508/cpsc508Project/polybench/polybench_compiled_tests/${TEST_NAMES[i]} ../result_logs/${TEST_NAMES[i]}.log > ../perf_results/${TEST_NAMES[i]}_perf.log
done
