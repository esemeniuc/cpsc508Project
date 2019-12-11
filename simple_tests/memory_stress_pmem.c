#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//This program allocates some number of gigabytes of longs and then randomly write and reads from them with zero locality
//Build to test if huge pages is installed correct. Would see new zero dTLB misses if 1GB pages are working.
//
//Use: compile and pass number of GB wanted, if no arguments are passed this defaults to 4GB can add 2nd paramater "quite" to prevent printing
//
//  ./a.out 
//  ./a.out 25
//  ./a.out 50 quite
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
int main(int argc, char **argv)
{
	int def = 4;
	long allocate = 1024;	
	int input;
	if (argc < 2){		
	 	allocate = ((long) def) * (1024*1024*1024) / sizeof (long); 
	}else if (input = atoi(argv[1])){
		allocate = ((long) input) * (1024*1024*1024) / sizeof (long); 
	}
	printf("attempting to allocate %ld bytes\n...  then will randomly write for ~5 seconds\n\n", allocate * sizeof(long));
	int fd = open("/dev/dax0.0", O_RDWR);
	if(fd == -1){printf("failed to open devdax\n"); exit(1);};
	//long *arr = mmap(NULL, allocate, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	long *arr = mmap(NULL, 19327352832, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);//x140000000);
	if(arr == MAP_FAILED){
		printf("mmap failed\n");
		exit(1);
	}
	for(int i = 0; i <MIN(allocate, 100*1000*1000) ; i++){
	        //print every now and then
		if(i % 10000 == 0 && argc < 3){
		       printf( "%ld", arr[rand() % (allocate -1)]);
		}
		//read forced with write 
		if(arr[rand() % (allocate - 1)] == rand() % 10){
			arr[i] = rand() % 10;		
		}
		//write
		arr[rand() % (allocate -1)] = i; 
	}
	free(arr);
}
