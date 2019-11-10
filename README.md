# cpsc508Project
CPSC 508/436C with Margo Seltzer

Idea list: https://docs.google.com/spreadsheets/d/1d5IcAig6RYBsYNjxQyyqtmS26Ah80Q3ZWhdd11BQCLo/edit#gid=0


### Determining & Managing Page Size

---

Run ```getconf PAGESIZE``` to check the current configured page size 
To configure page sizes, use the ```hugetlbfs```. https://lwn.net/Articles/375096/

To get hugetlbfs on Ubuntu, run:

```
sudo apt-get update
sudo apt-get install libhugetlbfs-dev
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

### Setting up emulated NVM

---

Good resource: https://pmem.io/2016/02/22/pm-emulation.html <br>
Step 1: Compile Linux kernel with options necessary for NVM emulation: run ```make nconfig``` and choose th appropriate options as described here: https://software.intel.com/en-us/articles/how-to-emulate-persistent-memory-on-an-intel-architecture-server <br>
Step 2: Run ```sudo update-grub``` to add in the new grub entry <br>
Step 3: Determine the free memory region for the memmap kernel param. for GRUB: https://nvdimm.wiki.kernel.org/how_to_choose_the_correct_memmap_kernel_parameter_for_pmem_on_your_system <br>
For us, this is: [mem 0x0000000100000000-0x000000087effffff], so we can start at 4G and allocate like 28 GB for pmem <br>
Step 4: Set the GRUB flag in grub.cfg ```memmap=ss[MKG]!nn[MKG]```

In order to facilitate easy booting between DRAM and PMEM systems: <br>
Install kexec stuff so we can boot between machines easily: <br>
sudo apt-get install kexec-tools <br>
PMEM bootflags: ```export BOOTFLAGS="cat /proc/cmdline memmap=28G\!4G"``` 

To boot into the PMEM kernel: 

```
sudo kexec -l /boot/vmlinuz-5.0.0-32-generic --initrd=/boot/initrd.img-5.0.0-32-generic --append=$BOOTFLAGS 
sudo systemctl kexec
```


When in the PMEM kernel, simple ```sudo shutdown -r now``` to boot back into DRAM kernel. 

Lastly, create and mount a DAX filesystem: <br>

```
mkdir /mnt/pmemdir
mkfs.ext4 /dev/pmem0
mount -o dax /dev/pmem0 /mnt/pmemdir
```

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
 
