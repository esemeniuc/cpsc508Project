#!/bin/sh

#python2 for hugeadm
sudo dnf install -y kexec-tools python2 perf ndctl redis psmisc @development-tools autoconf ccache

#backend for hugepages
wget -nc http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-2.20-8.fc29.x86_64.rpm

#hugeadm
wget -nc http://download-ib01.fedoraproject.org/pub/fedora/linux/releases/30/Everything/x86_64/os/Packages/l/libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm
sudo rpm -i libhugetlbfs-utils-2.20-8.fc29.x86_64.rpm

#git clone https://github.com/esemeniuc/cpsc508Project.git
#cd cpsc508Project

cd vmem || exit
make
sudo make install
