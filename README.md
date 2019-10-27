# cpsc508Project
CPSC 508/436C with Margo Seltzer

Idea list: https://docs.google.com/spreadsheets/d/1d5IcAig6RYBsYNjxQyyqtmS26Ah80Q3ZWhdd11BQCLo/edit#gid=0


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



#### Polybench

---




#### Postmark

---




#### Apex-MAP

---


