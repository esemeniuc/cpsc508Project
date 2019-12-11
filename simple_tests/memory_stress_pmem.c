#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//This program allocates some number of gigabytes of longs and then randomly write and reads from them with zero locality
//Build to test if huge pages is installed correct. Would see new zero dTLB misses if 1GB pages are working.
//
//Use: compile and pass number of GB wanted, if no arguments are passed this defaults to 4GB can add 2nd parameter "quiet" to prevent printing
//
//  ./a.out 
//  ./a.out 25
//  ./a.out 50 quiet
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int main(int argc, char **argv) {
    size_t allocate = 1024 * 1024 * 1024;
    if (argc == 3) {
        int input = atoi(argv[1]);
        allocate = (size_t) input * 1024 * 1024 * 1024;
    }
    printf("attempting to allocate %ld bytes\n...  then will randomly write for ~5 seconds\n\n", allocate);
    int fd = open("/dev/dax0.0", O_RDWR);
    if (fd == -1) {
        perror("failed to open devdax\n");
    };
    uint8_t *arr = mmap(NULL, allocate, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

//    uint_8t *arr = mmap(NULL, 19UL * 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);//x140000000);
    if (arr == MAP_FAILED) {
        perror("mmap failed\n");
    }
    for (int i = 0; i < MIN(allocate, 100 * 1024 * 1024); i++) {
        //print every now and then
        if (i % 100000 == 0 && argc < 3) {
            printf("%d", arr[rand() % allocate]);
        }
        //read forced with write
        if (arr[rand() % allocate] == rand() % 255) {
            arr[i] = rand() % 255;
        }
        //write
        arr[rand() % allocate] = i;
    }
    munmap(arr, 19UL * 1024 * 1024 * 1024);
}
