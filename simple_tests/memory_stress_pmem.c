#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//This program ns some number of gigabytes of longs and then randomly write and reads from them with zero locality
//Build to test if huge pages is installed correct. Would see new zero dTLB misses if 1GB pages are working.
//
//Use: compile and pass number of GB wanted, if no arguments are passed this defaults to 4GB can add 2nd parameter "quiet" to prevent printing
//
//  ./a.out 
//  ./a.out 25
//  ./a.out 50 quiet
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int main(int argc, char **argv) {
    size_t n = 4UL * 1024 * 1024 * 1024;
    if (argc == 3) {
        int input = atoi(argv[1]);
        n = (size_t) input * 1024 * 1024 * 1024;
    }
    printf("attempting to n %ld bytes\n...  then will randomly write for ~5 seconds\n\n", n);
    int fd = open("/dev/dax0.0", O_RDWR);
    if (fd == -1) {
        perror("failed to open devdax\n");
    };
    uint8_t *arr = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

//    uint_8t *arr = mmap(NULL, 19UL * 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);//x140000000);
    if (arr == MAP_FAILED) {
        perror("mmap failed\n");
    }
    for (int i = 0; i < MIN(n, 4UL * 1024 * 1024 * 1024); i++) {
        //print every now and then
        if (i % 100000 == 0 && argc < 3) {
            printf("%d", arr[rand() % n]);
        }
        //read forced with write
        size_t rand_num = ((size_t) rand() << 16) | (size_t) rand();
        if (arr[rand_num % n] % 10 < 5) { //50% chance of write
            arr[i] = rand_num;
        }
        //write
        arr[rand() % n] = i;
    }
    munmap(arr, n);
//    munmap(arr, 19UL * 1024 * 1024 * 1024);
}
