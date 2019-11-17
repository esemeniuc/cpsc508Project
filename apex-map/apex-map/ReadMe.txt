    Apex-Map assumes that each application or algorithm can be characterized by several major performance
factors. In Apex-Map 1.0, each data access stream has been characterized using three parameters: Memory 
size accessed, Spatial locality, and Temporal locality. By combining these three parameters together, 
Apex-Map can generate a synthetic stream to mimic the data access behavior.

    In version 3 of APEX-Map we implemented a more radical extension, which measures the performance influence 
of the arrangement and balance of functional units themselves. This allows APEX-Map to measure the 
computational performance of systems in addition to memory and network performance.  Using a code generator 
we produce different parameterized code segments to generate varying register pressures and computational 
intensities. The family of inner kernels is based on two new characteristic parameters: C: Computational 
Intensity and R: Register Pressure. In Apex-Map 3.0, we also characterize the loop structure (Nested Loop 
vs. Fused Loop) and store data effect (Scalar variables vs. Arrays). 

    Since we use a code generator in this version, the procedure to measure the performance becomes:
    1. Set the parameters in the input file (input)
    2. Run the generator to generate the benchmark code (Apex.c)
    3. Run the benchmark code to collect the performance results
    4. Using the filter (filter.c) to output the interested info, or you can examine the result files directly

    Following are the valid parameters and their allowed values (in CAPITAL LETTERS). Failing to provide 
correct values may cause unexpected program results. 

MODE: the test purpose of the code 
    -SEQUENTIAL:  to test sequential performance
    -MULTICORE: to test multicore effect  (need MPI for synchronization)
    -PARALLEL: to test parallel performance (not implemented)

ACCESS PATTERN: data access pattern
    -RANDOM:  need to define memory size, temporal locality, spatial locality

MEMORY SIZE: the memory size
    
SPATIAL LOCALITY: the list of spatial localities
    -1 -- MEMORY SIZE: could be defined in flexible manner, such as 1, 2, 4, 8-64 (a list starting from 8,
       ending at 64 in step *4)
 
TEMPORAL LOCALITY: the list of temporal locality
    -any value between [0,1], such as 0.1, 0.25, 1.0

ALGORITHM: how the loop structure is orchestrated
    -NESTED: using nested loop
    -FUSED: using one fused loop

INDEX SIZE: the number of indices generated in advance for each test
    -any number between 1 -- 1 Million
    
REGISTER PRESSURE: to test the performance effect of the number of registers
    -any positive integer (but it is usually set to a smaller number)

COMPUTATIONAL INTENSITY: to test the performance effect of computational intensity
    -any positive integer (but it is usually set to a smaller number)

REPEAT TIMES: the number of time to repeat the test
    -any positive integer, the larger the value, the more time the test takes, however, it should not be so 
     smaller that the test time is smaller than the clock resolution.

WARMUP TIMES: the number of times to warmup for each test pattern before timing
    -any positive integer, the larger the value

STORE METHOD: to decide how to store the left size data
    -ARRAY
    -SCALAR

CPU MHZ: the CPU frequency of the test platform
    -the cpu frequency of the test platform

PLATFORM: the name of the test platform
    -the name of the test platform

VERSION: the version of the benchmark
    -3.0

The default values for these parameters are:

MODE: SEQUENTIAL
ACCESS PATTERN: RANDOM
SPATIAL LOCALITY: 1-65536
TEMPORAL LOCALITY: 1.0 0.5 0.25 0.1 0.05 0.025 0.01 0.005 0.0025 0.001
MEMORY SIZE: 67108864
ALGORITHM: NESTED
REGISTER PRESSURE: 1
COMPUTATIONAL INTENSITY: 1
REPEAT TIMES: 100
WARMUP TIMES: 10
CPU MHZ: 2600
PLATFORM: TEST
VERSION: 3.0
INDEX SIZE: 1024
STORE METHOD: SCALAR

For convenience, a shell file is provided to perform a parameter sweep (sweep)

Work Todo:
1. test stride access pattern
2. add parallel implementation





