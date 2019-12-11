#!/bin/sh
sudo mkdir -p /mnt/ramdisk
sudo mount -t tmpfs -o size=20G tmpfs /mnt/ramdisk
perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk fio dram_rand_RW_1G.fio
sudo umount /mnt/ramdisk
