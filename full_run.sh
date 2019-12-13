#!/bin/bash
#usage: run from git root
#add "prototype" flag for fast iteration and no real pmem

set -x
SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
LOOP_COUNT=20
if [ "$2" == "prototype" ]; then LOOP_COUNT=1; fi

cd ${SCRIPTPATH} || exit #script expects to be in git root

if [ ! -f state_setup ]; then
    echo "RUNNING SETUP"
    #./setup.sh
    cat /proc/cmdline > backup_cmdline
    sudo tee /etc/systemd/system/benchmark.service <<EOF
[Unit]
Description=DRAM/PMEM Benchmark

[Service]
Type=simple
ExecStart=/bin/bash -c "${SCRIPT} &>> ${SCRIPTPATH}/output.txt"

[Install]
WantedBy=multi-user.target
EOF

    touch state_setup
    sudo systemctl enable benchmark.service

    if [ "$2" == "prototype" ]; then
        sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) memmap=32G!4G"
        sudo systemctl kexec
    else
        sudo systemctl enable --now benchmark
    fi
    exit 0 #avoid race until reboot kicks in
fi

#pmem
if [ ! -f state_pmem ]; then
    export HUGECTL_CMD=""
    echo "RUNNING 4K pmem"
    export TLB_SIZE="4K"
    sudo ndctl create-namespace -e namespace0.0 -m devdax -a 4K -f
    for _ in $(seq 1 $LOOP_COUNT); do
        ./pmem.sh
    done

    echo "RUNNING 2M pmem"
    export TLB_SIZE="2M"
    sudo ndctl create-namespace -e namespace0.0 -m devdax -a 2M -f
    for _ in $(seq 1 $LOOP_COUNT); do
        ./pmem.sh
    done

    echo "RUNNING 1G pmem"
    export TLB_SIZE="1G"
    sudo ndctl create-namespace -e namespace0.0 -m devdax -a 1G -f
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
    exit 0 #use to stop race until reboot kicks in
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


