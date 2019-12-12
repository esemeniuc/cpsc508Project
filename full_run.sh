#!/bin/bash
#20 gig pool sizes with hugeadm
set -x
SCRIPT=`realpath $0`
#SCRIPTPATH=`dirname $SCRIPT`

if [ ! -f state_setup ]; then
    echo "RUNNING SETUP"
    #./setup.sh
    cat /proc/cmdline > backup_cmdline
    sudo tee /etc/systemd/system/benchmark.service <<EOF
[Unit]
Description=DRAM/PMEM Benchmark

[Service]
Type=simple
ExecStart=/bin/bash ${SCRIPT}

[Install]
WantedBy=multi-user.target
EOF
#    sudo mv benchmark.service /etc/systemd/system/benchmark.service
#    sudo chown root:root /etc/systemd/system/benchmark.service
    sudo systemctl enable benchmark.service
    sudo touch state_setup
fi

#4K
if [ ! -f state_dram4K ]; then
    echo "RUNNING 4K"
    export HUGECTL_CMD=""
    export TLB_SIZE="4K"
    ./dram.sh
    sudo touch state_dram4K
    sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) default_hugepagesz=2M"
    sudo systemctl kexec
fi

#2M
if [ ! -f state_dram2M ]; then
    echo "RUNNING 2M"
    export HUGECTL_CMD="hugectl --heap=2097152"
    export TLB_SIZE="2M"
    sudo hugeadm --pool-pages-min 2M:10240
    ./dram.sh
    sudo touch state_dram2M
    sudo hugeadm --pool-pages-max 2M:0
    sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) default_hugepagesz=1G"
    sudo systemctl kexec
fi

#1G
if [ ! -f state_dram1G ]; then
    echo "RUNNING 1G"
    export HUGECTL_CMD="hugectl --heap=1073741824"
    export TLB_SIZE="1G"
    sudo hugeadm --pool-pages-min 1G:20
    ./dram.sh
    sudo touch state_dram1G
    sudo hugeadm --pool-pages-max 1G:0
    sudo kexec -l /boot/vmlinuz-$(uname -r) --initrd /boot/initramfs-$(uname -r).img --append "$(cat backup_cmdline) mmemmap=32G!4G"
    sudo systemctl kexec
fi

#sudo ndctl create-namespace -e namespace0.0 -m devdax -a 4K -f

#sudo kexec -l /boot/vmlinuz-5.3.12-300.local.fc31.x86_64 --initrd=/boot/initramfs-5.3.12-300.local.fc31.x86_64.img --append='BOOT_IMAGE=(hd0,gpt2)/vmlinuz-5.3.12-300.local.fc31.x86_64 root=UUID=24769c34-6aa9-4fe5-8be1-5e6fb7a289e2 ro rhgb quiet memmap=32G!4G'
#sudo systemctl kexec