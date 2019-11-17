#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define MHZ 2600.0
#define PLATFORM TESTPLATFORM
#define VERSION 3.0
/* change the following macro definition if high-resolution clock is available */
typedef struct timeval TIMESTRUCT;
#define CLOCK(t) gettimeofday(&t, 0)
#define TIMEDIFF(s, e, r) { \
  r = (e.tv_sec - s.tv_sec)+0.000001*(e.tv_usec-s.tv_usec); } 
#define MALLOC(size) malloc(size)
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)
#define MIN_N 4
#define MAXREG 1024

int MAXNCL = 1048576;
int R = 1;
int CI = 1;

int N = 100;
int L = 0;
double K = -1;
int I = 1024;
long long int B = 0;
double kavg;
int kflops, kmain, kindex, kdest;
int myid, nprocs;
long long int M, MM;
long long int dMem[] = {67108864};
double *data0;
double timeoverhead = 0.0;
double avg, sum2 = 0.0, flops = 0.0; 
double time3, time4;
TIMESTRUCT time1, time2, gtime1, gtime2;

long long int getIndex();
void initRand(int seed);
int initData(double *);
double test();
double gettimeoverhead();

int sLocality[] = {1,4,16,64,256,1024,4096,16384,65536};
double tLocality[] = {1.0000,0.5000,0.2500,0.1000,0.0500,0.0250,0.0100,0.0050,0.0025,0.0010};
long long int *ind;

int main(int argc, char *argv[]) {
  int i, j;
  time_t tval;
  int ccc = 1000000000;
  double ret = 0;
  long long int destBufLen, tmp;
  myid = 1;
  nprocs = 1;

  CLOCK(gtime1);
  N = 100;
  MM = 67108864, M = 67108864 / 1;
  data0 = (double *) MALLOC((M+0) * sizeof(double));
  if (data0 == NULL) { 
    printf("can not allcoate memory for data!\n");
     exit(1);
  }
  data0 = &data0[0];
  tmp = (long long int) 1024 * 65536;
  MAXNCL = MAX(MAXNCL, 65536);
  B = 1024; 
  ind = (long long int *) MALLOC(B * sizeof(long long int));
  if (ind == NULL) { 
    printf("can not allcoate memory for Ind and W!\n");
     exit(1);
  }

  initRand(myid);
  initData(data0);
  timeoverhead = gettimeoverhead();
  printf("Apex-Map SEQUENTIAL Test, Version 3.0 \n");
  printf("Platform : TESTPLATFORM, CPU Frequency : 2600.0(MHZ)\n");
  printf("Memory Size is %lld (Words), Register Pressure: %d \n", MM, R);
  printf("Repeat Times 100, Warmup Times 10, Computational Intensity: %d \n", CI);
  printf("Computational Kernel: NESTED loop, Implementation: SCALAR , Index Length: %d\n", I);
  time(&tval);
  printf("Start Time : %s\n", ctime(&tval));
  printf("------------------------------------------------------\n\n");

  for (i = 0; i < 9; i++) {
    L = sLocality[i];
    tmp = (long long int) I * L;
    B = MIN(MAXNCL, tmp); 
    for (j = 0; j < 10; j++) { 
      K = tLocality[j];
      ret = test();
      avg *= ccc; 
      sum2 *= ccc; 
      printf("K=%.4f, L=%d, B=%lld, ret = %f\n", K,L,B,ret); 
      printf("kernel: time(us), flops, access to data, index, destination: %.3f, %d, %d, %d, %d\n", kavg, kflops, kmain, kindex, kdest);
      printf("the average flops is %.3f(Mflops)\n", flops/1000000.0);
      printf("the average access time is %.3f(ns) with deviation %.3f ns\n", avg, sum2);
      printf("the average access time is %.3f(cycles) with deviation %.3f cylces, average bandwidth is %.3f(MB)/s\n\n\n", avg*MHZ/1000,sum2*MHZ/1000, 8*1000/avg); 
    }
  }
  free(data0);
  free(ind);
  printf("-----------------------------------------------------\n");
  CLOCK(gtime2);
  time(&tval);
  TIMEDIFF(gtime1, gtime2, ret);
  printf("Finished at : %s\n", ctime(&tval));
  printf("Total execution time : %.2f seconds\n", ret);
}
void initRand(int seed) {
  srand48(seed);
}
int initData(double *data) {
  long long int i;
  double inv = 0.1/M;
  for (i = 0; i < M; i++) {
    data[i] = i * inv;
  }
  return 0;
}
double gettimeoverhead() {
  TIMESTRUCT start, finish;
  double result, min = 1000000, avg, max = 0;
  int i;
  int n = 1000;

  avg = 0;
  for (i = 0; i < n; i++) {
    CLOCK(start);
    CLOCK(finish);
    TIMEDIFF(start, finish, result);
    if (result < min && result >= 0)
        min = result;
    if (result >  max)
        max = result;
    avg += result;
  }
  printf("Each call of the time function takes min %.9f seconds = %f cycles\n",
           min, min*MHZ*1000000.0);
  printf("Each call of the time function takes max %.9f seconds = %f cycles\n",
           max, max*MHZ*1000000.0);
  printf("Total %.9f, Average access time is %.9f seconds for %d times\n\n",
           avg, avg/n, n);
  return avg/n;
}
int initIndexArray(int length) {
  int i, j, k;
    for (i = 0; i < length; i++) {
        ind[i] = getIndex() * L;
    }
  return 0;
}
long long int getIndex() {
  return pow(drand48(), 1/K) * (M/L -1);
}
double test() {
  int i, j, k, v, m, times, length;
  int extra = 0;
  long long int jj, kk, pp;
  double sum = 0.0, t;
  double c0;
  double W0;
  W0 = 0.0;
  c0 = 0.1 + 0.01*0;
  time3 = 0;
  avg = 0;
  sum2 = 0;
  times = N;
  extra = 10;
  for (i = 0; i < times + extra; i++) {
    length = B / L;
    initIndexArray(length);
    CLOCK(time1);
    for (j = 0; j < length; j++) {
      for (k = 0; k < L; k++) {
        W0 = W0+c0*(data0[ind[j]+k]);
      }
    }
    if (i >= extra) { 
      CLOCK(time2);
      TIMEDIFF(time1, time2, time4);
      time4 -= timeoverhead;
      time3 += time4;
      sum2 += time4 * time4;
    }
  }
  t = time3 / times; kavg = t * 1000000; kflops = B * R * CI * 2; 
  flops = (B * R * CI * 2) / t;
  kmain = B * R; kdest = 0; 
  kindex = B / L * R;
  avg = time3 / (1.0 * times * R * (B));
  if (times > 1) { 
    sum2 = sqrt((sum2 - times*t*t)/(times-1))/(1.0*R*(B));
  }
  sum = W0;
  return sum;
}
