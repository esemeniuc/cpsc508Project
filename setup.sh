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

cd vmem/src/ || exit
make
sudo make install

