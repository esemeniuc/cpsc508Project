#!/bin/sh

sudo dnf install -y kexec-tools python2 perf ndctl redis psmisc @development-tools autoconf

#backend for hugepages
wget http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-2.20-8.fc29.x86_64.rpm

#hugeadm
wget http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm

#git clone https://github.com/esemeniuc/cpsc508Project.git
#cd cpsc508Project

cd vmem || exit
make
sudo make install

