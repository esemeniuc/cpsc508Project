#!/bin/bash
#usage: run from git root
#add "prototype" flag for fast iteration on emulated pmem

#this script creates temp files to remember its state
#have state for pmem, {4K,2M,1G} dram

set -x
SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)
LOOP_COUNT=5
if [ "$1" == "prototype" ]; then LOOP_COUNT=1; fi

cd ${SCRIPTPATH} || exit #script expects to be in git root

if [ ! -f state_setup ]; then
    echo "RUNNING SETUP"
    ./setup.sh
    if [ "$1" == "prototype" ]; then
        #shrink the problem size
        sed -i "s/16/1/" apex-map/apex_dram.sh apex-map/apex_pmem.sh
        sed -i "s/ 100 / 1 /" simple_tests/memory_stress_dram.c simple_tests/memory_stress_pmem.c
    fi

    cat /proc/cmdline > backup_cmdline
    sudo tee /etc/systemd/system/benchmark.service <<EOF
[Unit]
Description=DRAM/PMEM Benchmark

[Service]
Type=simple
ExecStart=/bin/bash -c "${SCRIPT} $1 &>> ${SCRIPTPATH}/output.txt"

[Install]
WantedBy=multi-user.target
EOF
    sudo systemctl enable benchmark
    touch state_setup

    #use emulated pmem
    if [ "$1" == "prototype" ]; then
        sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) memmap=32G!4G"
        sudo systemctl kexec
    else
        sudo systemctl start benchmark
    fi
    exit 0 #avoid race condition until reboot kicks in
fi

#pmem
if [ ! -f state_pmem ]; then
    export HUGECTL_CMD=""

    echo "RUNNING 4K pmem"
    export TLB_SIZE="4K"
    if [ "$1" == "prototype" ]; then
        sudo ndctl create-namespace -e namespace0.0 -m devdax -a 4K -f
    else
        export DAX_DEVICE="/dev/dax1.0"
    fi
    for _ in $(seq 1 $LOOP_COUNT); do
        ./pmem.sh
    done

    echo "RUNNING 2M pmem"
    export TLB_SIZE="2M"
    if [ "$1" == "prototype" ]; then
        sudo ndctl create-namespace -e namespace0.0 -m devdax -a 2M -f
    else
        export DAX_DEVICE="/dev/dax2.0"
    fi
    for _ in $(seq 1 $LOOP_COUNT); do
        ./pmem.sh
    done

    echo "RUNNING 1G pmem"
    export TLB_SIZE="1G"
    if [ "$1" == "prototype" ]; then
        sudo ndctl create-namespace -e namespace0.0 -m devdax -a 1G -f
    else
        export DAX_DEVICE="/dev/dax5.0"
    fi
    for _ in $(seq 1 $LOOP_COUNT); do
        ./pmem.sh
    done

    touch state_pmem
    sudo shutdown -r now
    exit 0
fi

#4K dram
if [ ! -f state_dram4K ]; then
    echo "RUNNING 4K dram"
    export HUGECTL_CMD=""
    export TLB_SIZE="4K"
    for _ in $(seq 1 $LOOP_COUNT); do
        ./dram.sh
    done

    touch state_dram4K
    sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) default_hugepagesz=2M"
    sudo systemctl kexec
    exit 0  #avoid race condition until reboot kicks in
fi

#2M dram
if [ ! -f state_dram2M ]; then
    echo "RUNNING 2M dram"
    export HUGECTL_CMD="hugectl --heap"
    export TLB_SIZE="2M"
    sudo hugeadm --pool-pages-min 2M:512
    sudo hugeadm --pool-pages-max 2M:20480 #allocate 40GB
    for _ in $(seq 1 $LOOP_COUNT); do
        ./dram.sh
    done
    touch state_dram2M
    sudo hugeadm --pool-pages-max 2M:0
    sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) default_hugepagesz=1G"
    sudo systemctl kexec
    exit 0
fi

#1G dram
if [ ! -f state_dram1G ]; then
    echo "RUNNING 1G dram"
    export HUGECTL_CMD="hugectl --heap"
    export TLB_SIZE="1G"
    sudo hugeadm --pool-pages-min 1G:25 #stop crashes for OOM
    for _ in $(seq 1 $LOOP_COUNT); do
        ./dram.sh
    done
    touch state_dram1G
    sudo hugeadm --pool-pages-max 1G:0
    exit 0
fi

echo "Finished running"
sudo systemctl disable benchmark