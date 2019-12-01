#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <sys/syscall.h>

int main() {
  int fd = open("/dev/dax0.0", O_RDWR);
  if (fd == -1) {
    printf("Fuck\n");
    printf("%s\n", strerror(errno));
  } else {
    printf("Opened fd %d\n", fd);
  }

 // void *vmem = mmap(NULL, 9663676416, 3,
//			MAP_SHARED, fd, 0);
  
  int *vmem = mmap(NULL, 19327352832, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (vmem == MAP_FAILED) { 
    printf("Error with mmap\n");
    printf("%s\n", strerror(errno));
    return 1;
  }

 
  for (int i = 0; i < 100; i++) {
	  char buf[1] = { i };
	  vmem[i] = buf[0];
  }
  
  for (int i = 0; i < 100; i++) {
	  printf("%d\n", vmem[i]);
  } 

  return 0;
}
