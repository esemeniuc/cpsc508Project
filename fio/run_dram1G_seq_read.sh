#!/bin/sh
sudo mkdir -p /mnt/ramdisk
sudo mount -t tmpfs -o size=20G tmpfs /mnt/ramdisk
sudo perf stat -B -d -d -d -e dtlb_load_misses.miss_causes_a_walk fio dram_seqr1G.fio
sudo umount /mnt/ramdisk
