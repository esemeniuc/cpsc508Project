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
    if (arr == MAP_FAILED) {
        perror("mmap failed\n");
    }

    for (int i = 0; i < 100 * 1000 * 1000; i++) {
        size_t r1 = rand(), r2 = rand();
        size_t rand_num1 = (r1 << 16) | r2;
        size_t rand_num2 = (r2 << 16) | r1;

        //print every now and then
        if (i % 100000 == 0 && argc < 3) {
            printf("%d", arr[rand_num1 % n]);
        }
        //read forced with write
        if (arr[rand_num2 % n] % 10 < 5) { //50% chance of write
            arr[rand_num2] = rand_num1;
        }
        //write
        arr[(rand_num1 ^ rand_num2) % n] = !(rand_num1 ^ rand_num2);
    }
    munmap(arr, n);
}
