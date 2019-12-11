#!/bin/bash

#apex
cd apex-map/apex-map || exit
./script.sh

#fio
cd fio || exit #script expects to be in directory
./run_dram_rand_RW_1G.sh
./run_dram_seq_R_1G.sh




#sudo ndctl create-namespace -e namespace0.0 -m devdax -a 4K -f

sudo kexec -l /boot/vmlinuz-5.3.12-300.local.fc31.x86_64 --initrd=/boot/initramfs-5.3.12-300.local.fc31.x86_64.img --append='BOOT_IMAGE=(hd0,gpt2)/vmlinuz-5.3.12-300.local.fc31.x86_64 root=UUID=24769c34-6aa9-4fe5-8be1-5e6fb7a289e2 ro rhgb quiet memmap=32G!4G'
sudo systemctl kexec