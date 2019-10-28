# cpsc508Project
CPSC 508/436C with Margo Seltzer

Idea list: https://docs.google.com/spreadsheets/d/1d5IcAig6RYBsYNjxQyyqtmS26Ah80Q3ZWhdd11BQCLo/edit#gid=0


### PERF

STD: ```perf run``` 

```sudo perf stat -e dTLB-loads-misses {benchmark command}```

NOTE: for Perf stat the following flags and data should be be recorded
 - dTLB-loads                                         
 - dTLB-load-misses                                   
 - dTLB-stores                                        
 - dTLB-store-misses                                  
 - dTLB-prefetches                                   
 - dTLB-prefetch-misses                              
 - iTLB-loads                                        
 - iTLB-load-misses                                  



### Reproduction steps to run tests

---


#### YCSB

---


Dependencies: openjdk8, python, maven <br>

Build steps: <br>

- Download ycsb bindings: ```curl -O --location https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz```
- Unzip: ```tar xfvz ycsb-0.17.0.tar.gz```
- Look at the appropriate binding README --> we will be using the Redis binding

For the Redis binding:

- Clone this: ```git clone http://github.com/brianfrankcooper/YCSB.git```
- ```cd YCSB```
- Then compile: ```mvn -pl site.ycsb:redis-binding -am clean package```
- To load the data: ```./bin/ycsb load redis -s -P workloads/workloada -p "redis.host=127.0.0.1" -p "redis.port=6379" > outputLoad.txt```
- To test: ```sudo perf stat -e dTLB-loads-misses ./bin/ycsb run redis -s -P workloads/workloada -p "redis.host=127.0.0.1" -p "redis.port=6379" > outputLoad.txt``` 

And there you have it!


#### Redis

---


- Start Redis Server: ```docker run -it --rm --name=redis redis:5-alpine```
- Start benchmark: ```docker run -it --rm --name=memtier --link=redis redislabs/memtier_benchmark -s 172.17.0.2```


#### Polybench

---


Dependencies: gcc/build tools <br>

Build steps: <br>

- Get Polybench: ```wget https://downloads.sourceforge.net/project/polybench/polybench-c-4.2.1-beta.tar.gz```
- Unzip: ```tar -xzf polybench-c-4.2.1-beta.tar.gz && cd polybench-c-4.2.1-beta.tar.gz```
- Compile a polybench benchmark(s): ```gcc -O3 -I utilities -I linear-algebra/kernels/atax utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -o atax_time```
- Run the compiled benchmark(s): (in this case) ```./atax_time```

Note, there are a bunch of different options that can be passed as macros using -D when compiling. There are also a whole bunch of different benchmarks for us to choose from. We will need to choose which benchmarks we want to run from the Polybench suite, along with what options to pass when compiling. 


#### Postmark

---

- Install: ```sudo apt install postmark```
- Run the benchmark: ```echo "set size 1000 9000
set number 50000
set transactions 100000
run" | sudo perf stat -e dTLB-load-misses,iTLB-load-misses postmark```



#### Apex-MAP
Apex-Map focuses on global data movement and measures how fast global data can be fed into computational units.

- Change your params in the apex-map/input file <br> 
-- Note: the MAXMEM flag is the number of DOUBLES that the benchmark will allocate, make sure that this will not exceed your <br>

Build steps: <br> <br>

- generate code to generate apex code ```gcc gen.pub.c -lm ``` <br>
- run code to generate code```./a.out``` <br>
- compile benchmark code ```gcc Apex.c -lm``` <br>
- run the benchmark```sudo perf stat -e dTLB-loads-misses ./a.out ``` <br>
 
