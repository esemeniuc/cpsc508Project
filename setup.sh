#!/bin/sh

sudo dnf install -y kexec-tools \
python2 \ #for hugeadm
perf \
ndctl \
redis \
psmisc \ #for killall
@development-tools

#backend for hugepages
wget http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-2.20-8.fc29.x86_64.rpm

#hugeadm
wget http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm

#To use hugeadm, run to check that you have 2MB and 1GB pages supported:
#hugeadm --pool-list 
#and listed as zero zero
#increase the min 2mb pages to 32GB of pages : sudo hugeadm --pool-pages-min 2MB:16384
#Make a new directory for your page size: sudo mkdir -p /mnt/hugetlbfs-2MB
#Then, mount: sudo mount -t hugetlbfs none -o pagesize=2MB

