#!/bin/bash
set -x
TLB_SIZES=(
"1G"
"2M"
"4K"
)

#20 gig pool sizes
HUGEADM_ALLOC_CMDS=(
"sudo hugeadm --pool-pages-min 1G:20"
"sudo hugeadm --pool-pages-min 2M:10240"
"" #none for 4K
)

HUGECTL_CMDS=(
"hugectl --heap=1073741824"
"hugectl --heap=2097152"
"" #no hugectl intercept needed for 4K
)

for ((i = 0; i < ${#TLB_SIZES[@]}; ++i)); do
    export HUGECTL_CMD=${HUGECTL_CMDS[i]}
    export TLB_SIZE=${TLB_SIZES[i]}

    #reset pools at each iteration
    sudo hugeadm --pool-pages-max 2M:0
    sudo hugeadm --pool-pages-max 1G:0
    if [ -n "${HUGEADM_ALLOC_CMDS[i]}" ]; then #don't run empty command
        bash -c ${HUGEADM_ALLOC_CMDS[i]}
    fi

    #apex
    cd apex-map/apex-map || exit
    ./apex_dram.sh
    cd ..

    #fio
    cd fio || exit #script expects to be in directory
    ./dram.sh
    cd ..

    #polybench
    cd polybench || exit
    ./compile_polybench.sh
    ./polybench_dram.sh
    rm -rf bin
    cd ..

    #simple_tests
    cd simple_tests || exit
    ./simple_dram.sh
    cd ..

done

#sudo ndctl create-namespace -e namespace0.0 -m devdax -a 4K -f

#sudo kexec -l /boot/vmlinuz-5.3.12-300.local.fc31.x86_64 --initrd=/boot/initramfs-5.3.12-300.local.fc31.x86_64.img --append='BOOT_IMAGE=(hd0,gpt2)/vmlinuz-5.3.12-300.local.fc31.x86_64 root=UUID=24769c34-6aa9-4fe5-8be1-5e6fb7a289e2 ro rhgb quiet memmap=32G!4G'
#sudo systemctl kexec