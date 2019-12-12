# Page of Enlightenment
CPSC 508/436C with Margo Seltzer

Idea list: https://docs.google.com/spreadsheets/d/1d5IcAig6RYBsYNjxQyyqtmS26Ah80Q3ZWhdd11BQCLo/edit#gid=0


### Determining & Managing Page Size

---

Run ```getconf PAGESIZE``` to check the current configured page size 
To configure page sizes, use the ```hugetlbfs```. https://lwn.net/Articles/375096/

To get hugetlbfs on Ubuntu, run:

```
sudo apt-get update
sudo apt-get install libhugetlbfs-dev hugepages
```

To check total number of huge pages: ```grep HugePages_Total /proc/meminfo``` <br>
To check the huge page size: ```grep Hugepagesize /proc/meminfo``` <br>
To check the number that are free: ```grep HugePages_Free /proc/meminfo``` <br>

To allocate hugepages manually ```echo <# of pages> | sudo tee /proc/sys/vm/nr_hugepages```


To view hugepage pools: ```hugeadm --pool-list``` <br>
To set the min pool page size: ```hugeadm --pool-pages-min 2MB:512``` <br>
To set the max pool page size: ```# hugeadm --pool-pages-max 2MB:2048``` 

To use libhugetlbfs features hugetlbfs must be mounted.  Each hugetlbfs mount point is associated with a page size.  To choose the size, use the pagesize mount option.  If this option is omitted, the default huge page size will be used.

Make a new directory for your page size: i.e. ```sudo mkdir -p /mnt/hugetlbfs-2MB``` <br>
Then, mount: ```sudo mount -t hugetlbfs none -o pagesize=2MB /mnt/hugetlbfs-2MB```

To get a full report of set up config: ```hugeadm --explain```

Launch applications with ```hugectl --heap <application>``` so hooks will be added to malloc calls, resulting in the use of huge pages

Eg for testing Apex-Map with 1GB TLB size
```
sudo kexec -l /boot/vmlinuz-5.0.3 --initrd=/boot/initrd.img-5.0.3 --append="root=UUID=7bc92aaf-86c1-4520-b473-a8064d22cb43 ro quiet splash $vt_handoff default_hugepagesz=1G"
sudo systemctl kexec
sudo hugeadm --pool-pages-min 1GB:10 #allocate 10 pages
sudo ~/linux-5.0.3/tools/perf/perf stat -ddd -x '|' hugectl --heap ./cpsc508Project/apex-map/apex-map/a.out
```

### Setting up emulated NVM
This system is based on Ubuntu 18.04.3 x64

Good resource: https://pmem.io/2016/02/22/pm-emulation.html <br>
Resource 2: https://software.intel.com/en-us/articles/how-to-emulate-persistent-memory-on-an-intel-architecture-server <br>

Step 1: Determine the free memory region for the memmap kernel param. for GRUB: https://nvdimm.wiki.kernel.org/how_to_choose_the_correct_memmap_kernel_parameter_for_pmem_on_your_system <br>
For us, this is: `[mem 0x0000000100000000-0x000000087effffff]`, so we can start at 4G and allocate like 28 GB for pmem <br>
Step 2: Create the kernel boot flag for your system in the format of `memmap=ss[MKG]!nn[MKG]`. For our system, we use `memmap=28G!4G`

In order to facilitate easy booting between DRAM and PMEM emulation: <br>
Install kexec stuff so we can boot between machines easily: <br>
```sudo apt-get install kexec-tools```

Step 3: Boot into PMEM emulation mode
```
export BOOTFLAGS="cat /proc/cmdline memmap=28G\!4G"
sudo kexec -l /boot/vmlinuz-5.0.0-32-generic --initrd=/boot/initrd.img-5.0.0-32-generic --append=$BOOTFLAGS 
sudo systemctl kexec
```
When in the PMEM kernel, simply run ```sudo shutdown -r now``` to boot back into DRAM system. 

Lastly, create and mount a DAX filesystem: <br>

```
sudo mkdir /mnt/pmem0
sudo mkfs.ext4 -b 4096 -E stride=262144 -F /dev/pmem0
sudo mount -o dax /dev/pmem0 /mnt/pmem0/
```

To run stuff on the emulated NVM, <br>
Install this stuff:
```sudo apt-get install libpmem1 librpmem1 libpmemblk1 libpmemlog1 libpmemobj1 libpmempool1``` <br>
Compile libvmmalloc: <br>
```sudo apt install libndctl-dev``` <br>

If you'd like to get actual memory usage stats for program running on the emulated PMEM: <br>
```LD_PRELOAD=/usr/local/lib/libvmmalloc.so valgrind --tool=massif /home/cs508/loop```

# TODO: Format for 2MB blocks 


### Install libvmmalloc
Notes: https://github.com/pmem/vmem
```
git clone https://github.com/pmem/vmem
cd vmem
git checkout tags/1.7
sudo apt install libndctl-dev libdaxctl-dev autoconf pkg-config
make
sudo make install
```

Demo:
```
export VMMALLOC_POOL_SIZE=$((27*1024*1024*1024))
export VMMALLOC_POOL_DIR="/mnt/pmemdir"

LD_PRELOAD=libvmmalloc.so.1 grep "pmem" /proc/mounts
```

Demo kernel build
`git clone git://kernel.ubuntu.com/ubuntu/ubuntu-bionic.git && cd ubuntu-bionic`
`git checkout Ubuntu-hwe-5.0.0-36.39_18.04.1`


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

- Start Redis: ```redis-server```
- Clone this: ```git clone http://github.com/brianfrankcooper/YCSB.git```
- ```cd YCSB```
- Then compile: ```mvn -pl site.ycsb:redis-binding -am clean package```
- To load the data: ```./bin/ycsb load redis -s -P workloads/workloada -p "redis.host=127.0.0.1" -p "redis.port=6379" > outputLoad.txt```
- To test: ```sudo perf stat -e dTLB-loads-misses ./bin/ycsb run redis -s -P workloads/workloada -p "redis.host=127.0.0.1" -p "redis.port=6379" > outputLoad.txt``` 

And there you have it! 


#### Redis

---

First, get memtier_benchmark:
- git clone https://github.com/RedisLabs/memtier_benchmark
Then, get dependencies
- sudo dnf install autoconf automake pkg-config libevent-devel pcre-devel
And build
- make

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




 ---
#### Apex-MAP
Apex-Map focuses on global data movement and measures how fast global data can be fed into computational units.

- Change your params in the apex-map/input file <br> 
-- Note: the MAXMEM flag is the number of DOUBLES that the benchmark will allocate, make sure that this will not exceed your memory<br>

Build steps: <br> <br>

- generate code to generate apex code ```gcc gen.pub.c -lm ``` <br>
- run code to generate code```./a.out``` <br>
- compile benchmark code ```gcc Apex.c -lm``` <br>
- run the benchmark```sudo perf stat -e dTLB-loads-misses ./a.out ``` <br>




#### Some documentation of APEX MAP

This is the source code we used, there is also some nice documentation basics on apex map here too <br>
https://crd.lbl.gov/departments/computer-science/PAR/research/previous-projects/apex/ <br>


Characterizing the Relation Between Apex-Map Synthetic Probes and Reuse Distance Distributions <br>
https://ieeexplore.ieee.org/document/5599180 <br>

	This paper has some nice math on the random access patterns of apex map as well as more details on the parameter effects

AdaptMemBench: Application-Specific MemorySubsystem Benchmarking
https://arxiv.org/pdf/1812.07778.pdf
	a more recent memory benchmarking tool that tries to do better than apex map


Hopscotch: A Micro-benchmark Suite for Memory Performance
https://github.com/alifahmed/hopscotch
	this paper has a great break down of memory pressure.
	

Apex-Map: A Synthetic Scalable Benchmark Probe to Explore Data Access Performance on Highly Parallel Systems (2005)
https://link.springer.com/chapter/10.1007/11549468_16
	og paper

Apex-Map: A Global Data Access Benchmark to Analyze HPC Systems and Parallel Programming Paradigms
https://ieeexplore.ieee.org/document/1560001
	og paper V2

