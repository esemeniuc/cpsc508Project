/*******************************************************************
The Apex-Map Performance Probe Generator (Apex-Map 3.0).
Developed by :
Erich Strohmaier, Hongzhang Shan (estrohmaier@lbl.gov, hshan@lbl.gov), 
Future Technology Group
Computational Research Division
Lawrence Berkeley National Laboratory
********************************************************************/
/******************************************************************* 
Main changes:  (02/10/2002)
1. Add two more architectural features:  Register Pressure, Computational Intensity 
2. Add the concept of programming styles: 
	a. Fused Loop implementation vs. Nested Loop implementation
	b. Using Scalar variables to store the results vs. Using Arrays 
3. Using generator to generate the test code
4. Change the kernel to perform multiply-add instead of add operation only
5. Enable the multicore test
********************************************************************/
/**************************TODO*************************************
0. Parallel Implementation
1. Stride Access
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* fin: parameter setting file, fout: the generated benchmark file name */
FILE *fout, *fin;
char buffer[1024];
char *retc;
/* the options to generate the benchmark, PARALLEL not implemented */
char *syms[]={
	"MODE", "PARALLEL LANGUAGE", "ACCESS PATTERN",
	"SPATIAL LOCALITY", "TEMPORAL LOCALITY", "MEMORY SIZE", 
	"ALGORITHM",  "REGISTER PRESSURE", "COMPUTATIONAL INTENSITY", 
	"REPEAT TIMES", "WARMUP TIMES", "INDEX SIZE", 
	"CPU MHZ", "PLATFORM", "VERSION",
	"STORE METHOD", "MAX CONCURRENT LOADS"};
		  
char fMode[40];			/* sequential, sequential multicore, or parallel */
char fParLang[20];		/* which parallel programming language to use for parallel implementation */
char fAlgo[20];			/* using NESTED loop or FUSED loop */
char fPattern[20];		/* using RANDOM stream or STRIDE ACCESS stream */ 
int  dSpatial[40];      /* the spatial locality of defined random access stream */
int  nSpatial, nTemporal, nMemSize, nStride; /* the number of tested values for each paramter */
int  dStride[40];       /* the strides for STRIDE access stream */
double dTemporal[40];   /* the temporal locality for defined random access streams */
long long int dMemSize[40];  /* the memory sizes to use */
int nMaxCurLoads, nReg, nCI, N=1024, dWarmup=0;
long long int dMaxCurLoads[40]; /* the maximum number of current independent loads */
int dReg[40], dCI[40];          /* the register pressures and computational intensities */
double dMHZ;		    /* the CPU speed */
char fPlatform[100];    /* the tested platform name */
char fVersion[10];      /* the benchmark version */
int dUnroll = 1;        /* how to unroll the loops */ 
int fRandom = 1;        /* random stream flag */
int dStrideBlock;       /* the block size for STRIDE access */
int dIndex;             /* the index size */
char fStore[20];        /* the store method for the left-side data */
char fSweepB = 0;       /* if B(MaxCurLoads) is defined, whether to test the performance using different block size */

#define NSYM 17
/* The MaxCurLoads is limited to MAXNCL */
int MAXNCL= 1048576;

/* prome generator */           
int prime() {
  int i, j, n, next;
  static last = 11;

  next = last + 2;
  while (1) {
    n = sqrt(next * 1.0);
    for (i = 3; i <= n; i += 1) {
        if (next % i == 0) break;
    }
    if (i > n) {
        last = next; break;
    }
    else {
        next += 2;
    }
  }
  return last;
}

/* read in parameters */
int findSel(char *buf) {
  int pos = 0, i = 0, j;
  char sym[64], c;
  int num0, num1, p1, range, n;
  double dnum0;
  long long int lnum0, lnum1, ln;

  while ((buf[i] != ':') && (buf[i] != '\n')) {sym[i] = buf[i]; i++;}
  sym[i] = '\0';
  if (buf[i] == '\n') {
	printf("Input format is not right, no : for %s\n", buf);
    exit(1);
  }
  
  while ((pos < NSYM) && (strcmp(syms[pos],sym))) pos++;
  if (pos >= NSYM) {
    printf("Input Parameter Error, No Such Parameter: %s\n", sym);
    exit(1);
  }

  i++;
  switch (pos) {
    case 0:
		/* MODE: sequential, multicore, or parallel, default: sequential */
        while (buf[i] == ' ') i++;
		j = 0;
		while (buf[i] != '\n') fMode[j++] = buf[i++];
		fMode[j] = '\0';
		if ((strncmp(fMode, "SEQUENTIAL", 10) != 0) && (strncmp(fMode, "MULTICORE", 9) != 0) &&
	      (strncmp(fMode, "PARALLEL", 8) != 0)) {
			printf("No Such Mode is Implemented: %s\n", fMode);
			exit(1);
		}
		break;
    case 1:
		/* PARALLEL LANGUAGE: which parallel language */
		if (strncmp(fMode, "PARALLEL", 8) == 0) { 
			while (buf[i] == ' ') i++;
			strcpy(fParLang, &buf[i]);
			if ((strncmp(fParLang, "MPI", 3) != 0) && (strncmp(fParLang, "SHMEM", 5) != 0) &&
				(strncmp(fParLang, "UPC", 3) != 0) && (strncmp(fParLang, "CAF", 3) != 0)) {
				printf("No Such Parallel Language is Implemented: %s\n", fParLang);
				exit(1);
			}
		}
		break;
    case 2:
		/* ACCESS PATTERN: RANDOM/STRIDE */
        while (buf[i] == ' ') i++;
		j = 0;
		while (buf[i] != '\n') fPattern[j++] = buf[i++];
		fPattern[j] = '\0';
        if ((strncmp(fPattern, "RANDOM", 6) != 0) && (strncmp(fPattern, "STRIDE", 6) != 0)) {
          printf("No Such Access Pattern is Implemented: %s\n", fPattern);
          exit(1);
        }
		if (strncmp(fPattern, "RANDOM", 6) == 0)  fRandom = 1;
        break;
    case 3:
		/* SPATIAL LOCALITY: for example, 1,2, 4-64, 128 */
		nSpatial = 0;
		range = 0;
		while (buf[i] != '\n') {
			while (buf[i] == ' ') i++;
			if (buf[i] == '\n') break;
			p1 = i;
			while (isdigit(buf[i])) i++;
			c = buf[i];
			buf[i] = '\0';
			num1 = atoi(&buf[p1]);
			buf[i] = c;
	  
			if (c == '-') {
				range = 1;
				num0 = num1;
				buf[i] = ' ';
			}
			else {
				if (range) {
					range = 0;
					for (n = num0; n <= num1; n *= 4) {
						dSpatial[nSpatial++] = n;
					}
				}
				else {
					dSpatial[nSpatial++] = num1;
				}
			}
		}
		break;
    case 4:
        /* TEMPORAL LOCALITY: lis of tempral localities [0--1] */
        nTemporal = 0;
        range = 0;
        while (buf[i] != '\n') {
          while (buf[i] == ' ') i++;
          if (buf[i] == '\n') break;
          p1 = i;
          while (isdigit(buf[i]) || buf[i] == '.') i++;
          c = buf[i];
          buf[i] = '\0';
          dnum0 = atof(&buf[p1]);
          buf[i] = c;
          dTemporal[nTemporal++] = dnum0;
        }
        break;
    case 5:
        /* MEMORY SIZE:  */
        nMemSize = 0;
		dMemSize[0] = 67108864;  /* default value */
		
		while (buf[i] == ' ') i++;
        if (buf[i] == '\n') break;
        p1 = i;
        while (isdigit(buf[i])) i++;
        buf[i] = '\0';
        dMemSize[nMemSize++] = strtoll(&buf[p1], NULL, 10);
        break;		
    case 6:
        /* LOOP ALGORITHM: NESTED LOOP, FUSED LOOP */
        while (buf[i] == ' ') i++;
		j = 0;
		while (buf[i] != '\n') fAlgo[j++] = buf[i++];
		fAlgo[j] = '\0';
        if ((strncmp(fAlgo, "FUSED", 5) != 0) && (strncmp(fAlgo, "NESTED", 6) != 0) &&
            (strncmp(fAlgo, "INVNESTED", 9) != 0)) {
          printf("No Such Algorithm is Implemented: %s\n", fAlgo);
          exit(1);
        }
        break;
    case 7:
        /* Number of Register Pressure */
        nReg = 0;
		while (buf[i] == ' ') i++;
        if (buf[i] == '\n') break;
        p1 = i;
        while (isdigit(buf[i])) i++;
        buf[i] = '\0';
        dReg[nReg++] = atoi(&buf[p1]);
        break;		

    case 8:
        /* Computational Intensity */
        nCI = 0;
		while (buf[i] == ' ') i++;
        if (buf[i] == '\n') break;
        p1 = i;
        while (isdigit(buf[i])) i++;
        buf[i] = '\0';
        dCI[nCI++] = atoi(&buf[p1]);
        break;		
    case 9:
    case 10:
        /* EXPERIMENT REPEAT TIMES */
        while (buf[i] == ' ') i++;
        p1 = i;
        while (isdigit(buf[i])) i++;
        c = buf[i];
        buf[i] = '\0';
        if (pos == 9) 
	    N = atoi(&buf[p1]);
	else
            dWarmup = atoi(&buf[p1]);
	break;
    case 11:
        /* Index Size */
        while (buf[i] == ' ') i++;
        if (buf[i] == '\n') break;
        p1 = i;
        while (isdigit(buf[i])) i++;
        c = buf[i];
        buf[i] = '\0';
        dIndex = atoi(&buf[p1]);
        break;
    case 12:
		/* CPU SPEED */
		while (buf[i] == ' ') i++;
		dMHZ = atof(&buf[i]);
		break;
    case 13:
        /* PLATFORM */
        while (buf[i] == ' ') i++;
		j = 0;
		while (buf[i] != '\n') fPlatform[j++] = buf[i++];
		fPlatform[j] = '\0';
        break;
    case 14:
        /* VERSION */
        while (buf[i] == ' ') i++;
		j = 0;
		while (buf[i] != '\n') fVersion[j++] = buf[i++];
		fVersion[j] = '\0';
        break;
    case 15:
        /* Store Method of Left hand-Size Values */
        while (buf[i] == ' ') i++;
        j = 0;
        while (buf[i] != '\n') fStore[j++] = buf[i++];
        fStore[j] = '\0';
        break;
	case 16:
        /* Maximum Number of Concurrent Loads */
        nMaxCurLoads = 0;
		while (buf[i] == ' ') i++;
        if (buf[i] == '\n') break;
        p1 = i;
        while (isdigit(buf[i])) i++;
        buf[i] = '\0';
        dMaxCurLoads[nMaxCurLoads++] = strtoll(&buf[p1], NULL, 10);
        break;		
    default:
	    printf("Nondefined Keywords!\n");
		break;      
  }
  return pos;
}

void printInfo() {
  int i;

  printf("MODE: %s\n", fMode);
  if (strncmp(fMode, "PARALLEL", 8) == 0) 
	printf("PARALLEL LANGUAGE: %s\n", fParLang);

  printf("DATA ACCESS PATTERN: %s\n", fPattern);
  if (strncmp(fPattern, "RANDOM", 6) == 0) {
	printf("Spatial Locality: ");
	for (i = 0; i < nSpatial; i++)
		printf("%d ", dSpatial[i]);
	printf("\n");

	printf("Temporal Locality: ");
	for (i = 0; i < nTemporal; i++)
		printf("%.4f ", dTemporal[i]);
	printf("\n");
  }
  else {
	printf("UNDEFINED DATA ACCESS PATTERN: %s\n", fPattern);
  }
  printf("ALGORITHM: %s\n", fAlgo);
  if (dMaxCurLoads) {
    printf("MAX CONCURRENT LOADS: ");
    for (i = 0; i < nMaxCurLoads; i++)
		printf("%lld ", dMaxCurLoads[i]);
  }
  
  printf("Mem Size (W): ");
  for (i = 0; i < nMemSize; i++)
    printf("%lld ", dMemSize[i]);
  printf("\n");

  printf("REGISTERS PRESSURE: ");
  for (i = 0; i < nReg; i++)
    printf("%d ", dReg[i]);
  printf("\n");

  printf("COMPUTATIONAL INTENSITY: ");
  for (i = 0; i < nCI; i++)
    printf("%d ", dCI[i]);
  printf("\n");

  printf("REPEAT TIMES: %d\n", N);
  printf("WARMUP TIMES: %d\n", dWarmup);

  printf("CPU SPEED: %.1f\n", dMHZ);
  printf("PLATFORM: %s\n", fPlatform);
  printf("VERSION: %s\n", fVersion);
  
  printf("INDEX SIZE: %d\n\n", dIndex);
  printf("STORE METHOD: %s\n\n",  fStore);

  if (nSpatial && nMaxCurLoads) {
    printf("Warning: both L and B are defined. Only L is used!\n");
  }
}

void createFFile() {
  printf("Not Implemented!\n");
}

void createCFile() {
  int i, j, k;
  int t, tt;
  long long int maxMem;

  if (strncmp(fMode, "PARALLEL", 8) == 0) {
	printf("Parallel Version Not Implemented Yet!\n");
	return;
  }
  fout = fopen("Apex.c", "w+");
  if (fout == NULL) {
    printf("Could not create file Apex.c\n");
    return;
  }

  fprintf(fout, "#include <stdio.h>\n");
  fprintf(fout, "#include <stdlib.h>\n");
  fprintf(fout, "#include <math.h>\n");
  fprintf(fout, "#include <time.h>\n");
  fprintf(fout, "#include <sys/time.h>\n");

  fprintf(fout, "\n#define MHZ %.1f\n", dMHZ);
  fprintf(fout, "#define PLATFORM %s\n", fPlatform);
  fprintf(fout, "#define VERSION %s\n", fVersion);
  
  if (strncmp(fMode, "SEQUENTIAL", 9) == 0) {  
	fprintf(fout, "/* change the following macro definition if high-resolution clock is available */\n");
	fprintf(fout, "typedef struct timeval TIMESTRUCT;\n");
	fprintf(fout, "#define CLOCK(t) gettimeofday(&t, 0)\n");
	fprintf(fout, "#define TIMEDIFF(s, e, r) { \\\n");
	fprintf(fout, "  r = (e.tv_sec - s.tv_sec)+0.000001*(e.tv_usec-s.tv_usec); } \n");
  }
  else {
    if ((strncmp(fMode, "MULTICORE", 9) == 0) ||
	    ((strncmp(fMode, "PARALLEL", 8) == 0) && (strncmp(fParLang, "MPI", 3)==0))) {
		fprintf(fout, "#include <mpi.h>\n"); 
		fprintf(fout, "typedef double TIMESTRUCT;\n");
		fprintf(fout, "#define CLOCK(t) t = MPI_Wtime();\n");
		fprintf(fout, "#define TIMEDIFF(s, e, r) r = e - s;\n"); 
    }  
  }
  	
  fprintf(fout, "#define MALLOC(size) malloc(size)\n"); 
  fprintf(fout, "#define MAX(a, b) (a > b ? a : b)\n");
  fprintf(fout, "#define MIN(a, b) (a > b ? b : a)\n");
  fprintf(fout, "#define MIN_N 4\n");
  fprintf(fout, "#define MAXREG 1024\n\n");
  fprintf(fout, "int MAXNCL = 1048576;\n");

  fprintf(fout, "int R = %d;\n", dReg[0]);
  fprintf(fout, "int CI = %d;\n", dCI[0]);

  fprintf(fout, "\nint N = %d;\n", N);
  if (strncmp(fPattern, "RANDOM", 6)) {
    fprintf(fout, "int S = 0;\n");
    fprintf(fout, "long long int B = %lld;\n", dMemSize[0]/dStride[0]*dStrideBlock);
  }
  else {
    fprintf(fout, "int L = 0;\n");
    fprintf(fout, "double K = -1;\n");
    fprintf(fout, "int I = %d;\n", dIndex);
    fprintf(fout, "long long int B = %lld;\n", dMaxCurLoads[0]);
  }
  fprintf(fout, "double kavg;\n");
  fprintf(fout, "int kflops, kmain, kindex, kdest;\n");
  fprintf(fout, "int myid, nprocs;\n");
  fprintf(fout, "long long int M, MM;\n");
  fprintf(fout, "long long int dMem[] = {%lld", dMemSize[0]);
  for (i = 1; i < nMemSize; i++) {
    fprintf(fout, ", %lld", dMemSize[i]);
  }  
  fprintf(fout, "};\n");

  for (i = 0; i < dReg[0]; i++)
    fprintf(fout, "double *data%d;\n", i);
  fprintf(fout, "double timeoverhead = 0.0;\n");
  fprintf(fout, "double avg, sum2 = 0.0, flops = 0.0; \n");
  fprintf(fout, "double time3, time4;\n");
  fprintf(fout, "TIMESTRUCT time1, time2, gtime1, gtime2;\n\n");

  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    fprintf(fout, "long long int getIndex();\n");
    fprintf(fout, "void initRand(int seed);\n");
  }
  fprintf(fout, "int initData(double *);\n");
  fprintf(fout, "double test();\n");
  fprintf(fout, "double gettimeoverhead();\n");
  
  fprintf(fout, "\n");
  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    fprintf(fout, "int sLocality[] = {%d", dSpatial[0]);
    for (i = 1; i < nSpatial; i++)
      fprintf(fout, ",%d", dSpatial[i]);
    fprintf(fout, "};\n");
 
    fprintf(fout, "double tLocality[] = {%.4f", dTemporal[0]);
    for (i = 1; i < nTemporal; i++)
      fprintf(fout, ",%.4f", dTemporal[i]);
    fprintf(fout, "};\n");

    fprintf(fout, "long long int *ind;\n");
  }

  if (strncmp(fStore, "ARRAY", 5) == 0) {
    for (i = 0; i < dReg[0]; i++) {
      fprintf(fout, "double *W%d;\n", i);
    }
  }

  fprintf(fout, "\nint main(int argc, char *argv[]) {\n");
  fprintf(fout, "  int i, j;\n");
  fprintf(fout, "  time_t tval;\n");
  fprintf(fout, "  int ccc = 1000000000;\n");
  fprintf(fout, "  double ret = 0;\n");
  fprintf(fout, "  long long int destBufLen, tmp;\n");

  if ((strncmp(fMode, "MULTICORE", 9) == 0) ||
      ((strncmp(fMode, "PARALLEL", 8) == 0) && (strncmp(fParLang, "MPI", 3)==0))) {
	fprintf(fout, "  MPI_Init(&argc, &argv);\n");
	fprintf(fout, "  MPI_Comm_rank(MPI_COMM_WORLD, &myid);\n");
	fprintf(fout, "  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);\n");
  }
  else {
    fprintf(fout, "  myid = 1;\n");
	fprintf(fout, "  nprocs = 1;\n");
  }	
	  
  fprintf(fout, "\n");
  fprintf(fout, "  CLOCK(gtime1);\n");
  fprintf(fout, "  N = %d;\n", N);
  maxMem = dMemSize[0];
  for (i = 1; i < nMemSize; i++) {
    if (dMemSize[i] > maxMem)  maxMem = dMemSize[i];
  }
  fprintf(fout, "  MM = %lld, M = %lld / %d;\n", maxMem, maxMem, dReg[0]);
  for (i = 0; i < dReg[0]; i++) { 
    j = (i == 0 ? 0  : prime());
    fprintf(fout, "  data%d = (double *) MALLOC((M+%d) * sizeof(double));\n", i, j);
    fprintf(fout, "  if (data%d == NULL) { \n", i);
    fprintf(fout, "    printf(\"can not allcoate memory for data!\\n\");\n ");
    fprintf(fout, "    exit(1);\n");
    fprintf(fout, "  }\n");
    fprintf(fout, "  data%d = &data%d[%d];\n", i, i, j);
  }


  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    if (nSpatial) {
        if (dSpatial[0] == 0) 
			fprintf(fout, "  tmp = MAXNCL =(long long int) %d * %d;\n", dIndex, maxMem);
        else
			fprintf(fout, "  tmp = (long long int) %d * %d;\n", dIndex, dSpatial[nSpatial-1]);
	    fprintf(fout, "  MAXNCL = MAX(MAXNCL, %d);\n", dSpatial[nSpatial -1]);
    	if (strncmp(fAlgo, "NESTED", 6) == 0) 
			fprintf(fout, "  B = %d; \n", dIndex);
		else
			fprintf(fout, "  B = MIN(MAXNCL, tmp); \n");
    }
    else {
		fprintf(fout, "  B = %lld; \n", dMaxCurLoads[nMaxCurLoads-1]);
        fprintf(fout, "  destBufLen = B;\n");
    }
   
    if (strncmp(fStore, "ARRAY", 5) == 0) {
		if (nSpatial) 
          fprintf(fout, "  destBufLen = MIN(MAXNCL, tmp); \n");
		else
          fprintf(fout, "  destBufLen = %lld; \n", dMaxCurLoads[nMaxCurLoads-1]);
        for (i = 0; i < dReg[0]; i++) {
          fprintf(fout, "  W%d = (double *) MALLOC(destBufLen * sizeof(double));\n", i);
        }
    }
    fprintf(fout, "  ind = (long long int *) MALLOC(B * sizeof(long long int));\n", i);
    fprintf(fout, "  if (ind == NULL) { \n");
    fprintf(fout, "    printf(\"can not allcoate memory for Ind and W!\\n\");\n ");
    fprintf(fout, "    exit(1);\n");
    fprintf(fout, "  }\n");

    fprintf(fout, "\n  initRand(myid);\n");
  }
  
  for (i = 0; i < dReg[0]; i++)
    fprintf(fout, "  initData(data%d);\n", i);
  fprintf(fout, "  timeoverhead = gettimeoverhead();\n");
  
  if (strncmp(fMode, "MULTICORE", 9) == 0) {
	fprintf(fout, "  printf(\"Proc: %%d, Apex-Map %s Test, Version %s \\n\", myid);\n", fMode, fVersion);
	fprintf(fout, "  printf(\"Proc: %%d, Platform : %s, CPU Frequency : %.1f(MHZ)\\n\", myid);\n", fPlatform, dMHZ);
	fprintf(fout, "  printf(\"Proc: %%d, Memory Size is %%lld (Words), Register Pressure: %%d \\n\", myid, MM, R);\n"); 
	fprintf(fout, "  printf(\"Proc: %%d, Repeat Times %d, Warmup Times %d, Computational Intensity: %%d \\n\", myid, CI);\n", N, dWarmup);
	if (strncmp(fPattern, "RANDOM", 6) == 0) {
		fprintf(fout, "  printf(\"Proc: %%d, Computational Kernel: %s loop, Implementation: %s , Index Length: %%d\\n\", myid, I);\n", fAlgo, fStore);
		if (nSpatial == 0) 
			fprintf(fout, "  printf(\"Proc: %%d, Max Current Number Loads is %lld \\n\", myid);\n", dMaxCurLoads[0]);
	}
  }
  else {
	fprintf(fout, "  printf(\"Apex-Map %s Test, Version %s \\n\");\n", fMode, fVersion);
	fprintf(fout, "  printf(\"Platform : %s, CPU Frequency : %.1f(MHZ)\\n\");\n", fPlatform, dMHZ);
	fprintf(fout, "  printf(\"Memory Size is %%lld (Words), Register Pressure: %%d \\n\", MM, R);\n"); 
	fprintf(fout, "  printf(\"Repeat Times %d, Warmup Times %d, Computational Intensity: %%d \\n\", CI);\n", N, dWarmup);
	if (strncmp(fPattern, "RANDOM", 6) == 0) {
		fprintf(fout, "  printf(\"Computational Kernel: %s loop, Implementation: %s , Index Length: %%d\\n\", I);\n", fAlgo, fStore);
		if (nSpatial == 0) 
			fprintf(fout, "  printf(\"Max Current Number Loads is %lld \\n\");\n", dMaxCurLoads[0]);
	}
  }
  fprintf(fout, "  time(&tval);\n");
  if (strncmp(fMode, "MULTICORE", 9) == 0) 
	fprintf(fout, "  printf(\"Proc: %%d, Start Time : %%s\\n\", myid, ctime(&tval));\n");
  else
    fprintf(fout, "  printf(\"Start Time : %%s\\n\", ctime(&tval));\n");
  
  fprintf(fout, "  printf(\"------------------------------------------------------\\n\\n\");\n\n");

  if (strncmp(fPattern, "RANDOM", 6) == 0) {
      if (nSpatial) {
        fprintf(fout, "  for (i = 0; i < %d; i++) {\n", nSpatial);
        fprintf(fout, "    L = sLocality[i];\n");
        fprintf(fout, "    tmp = (long long int) I * L;\n");
        fprintf(fout, "    B = MIN(MAXNCL, tmp); \n");
      }
      else {
		  fprintf(fout, "  for (L = B; L <= B; L *= 2) {\n");
      }
	  
	  fprintf(fout, "    for (j = 0; j < %d; j++) { \n", nTemporal);
      fprintf(fout, "      K = tLocality[j];\n");
	  
	  if (strncmp(fMode, "MULTICORE", 9) == 0) {
		fprintf(fout, "      MPI_Barrier(MPI_COMM_WORLD);\n");
	  }
	  	  
      fprintf(fout, "      ret = test();\n");
      fprintf(fout, "      avg *= ccc; \n");
      fprintf(fout, "      sum2 *= ccc; \n");
	  if (strncmp(fMode, "MULTICORE", 9) == 0) {
		fprintf(fout, "      printf(\"Proc: %%d, K=%%.4f, L=%%d, B=%%lld, ret = %%f\\n\", myid, K,L,B,ret); \n");
		fprintf(fout, "      printf(\"Proc: %%d, kernel: time(us), flops, access to data, index, destination:");
		fprintf(fout, " %%.3f, %%d, %%d, %%d, %%d\\n\", myid, kavg, kflops, kmain, kindex, kdest);\n"); 
		fprintf(fout, "      printf(\"Proc: %%d, the average flops is %%.3f(Mflops)\\n\", myid, flops/1000000.0);\n");
		fprintf(fout, "      printf(\"Proc: %%d, the average access time is %%.3f(ns) with deviation %%.3f ns\\n\", myid, avg, sum2);\n");
		fprintf(fout, "      printf(\"Proc: %%d, the average access time is %%.3f(cycles) with deviation %%.3f cylces");
		fprintf(fout, ", average bandwidth is %%.3f(MB)/s\\n\\n\\n\", myid, avg*MHZ/1000,sum2*MHZ/1000, 8*1000/avg); \n");	  
	  }
	  else {
		fprintf(fout, "      printf(\"K=%%.4f, L=%%d, B=%%lld, ret = %%f\\n\", K,L,B,ret); \n");
		fprintf(fout, "      printf(\"kernel: time(us), flops, access to data, index, destination:");
		fprintf(fout, " %%.3f, %%d, %%d, %%d, %%d\\n\", kavg, kflops, kmain, kindex, kdest);\n"); 
		fprintf(fout, "      printf(\"the average flops is %%.3f(Mflops)\\n\", flops/1000000.0);\n");
		fprintf(fout, "      printf(\"the average access time is %%.3f(ns) with deviation %%.3f ns\\n\", avg, sum2);\n");
		fprintf(fout, "      printf(\"the average access time is %%.3f(cycles) with deviation %%.3f cylces");
		fprintf(fout, ", average bandwidth is %%.3f(MB)/s\\n\\n\\n\", avg*MHZ/1000,sum2*MHZ/1000, 8*1000/avg); \n");
	  }
      fprintf(fout, "    }\n");
      fprintf(fout, "  }\n");
  }

  for (i = 0; i < dReg[i]; i++) {
    fprintf(fout, "  free(data%d);\n", i);
  }

  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    fprintf(fout, "  free(ind);\n");
  }

  if (strncmp(fStore, "ARRAY", 5) == 0) {
    for (i = 0; i < dReg[0]; i++) {
      if (strncmp(fStore, "ARRAY", 5) == 0) {
        fprintf(fout, "  free(W%d);\n", i);
      }
    }
  }
  fprintf(fout, "  printf(\"-----------------------------------------------------\\n\");\n");
  fprintf(fout, "  CLOCK(gtime2);\n");
  fprintf(fout, "  time(&tval);\n");
  fprintf(fout, "  TIMEDIFF(gtime1, gtime2, ret);\n");
  
  if (strncmp(fMode, "MULTICORE", 9) == 0) {
	fprintf(fout, "  printf(\"Proc: %%d, Finished at : %%s\\n\", myid, ctime(&tval));\n");
	fprintf(fout, "  printf(\"Proc: %%d, Total execution time : %%.2f seconds\\n\", myid, ret);\n");  
  	fprintf(fout, "  MPI_Finalize();\n");
  }
  else {
	fprintf(fout, "  printf(\"Finished at : %%s\\n\", ctime(&tval));\n");
	fprintf(fout, "  printf(\"Total execution time : %%.2f seconds\\n\", ret);\n");
  }
  
  fprintf(fout, "}\n");


  /**** function initRand() ****/
  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    fprintf(fout, "void initRand(int seed) {\n");
    fprintf(fout, "  srand48(seed);\n");
    fprintf(fout, "}\n");
  }
  /**** function initData() ****/
  fprintf(fout, "int initData(double *data) {\n");
  fprintf(fout, "  long long int i;\n");
  fprintf(fout, "  double inv = 0.1/M;\n");
  fprintf(fout, "  for (i = 0; i < M; i++) {\n");
  fprintf(fout, "    data[i] = i * inv;\n");
  fprintf(fout, "  }\n");
  fprintf(fout, "  return 0;\n");
  fprintf(fout, "}\n");

  /**** gettiemoverhead(), may chnage the implementation ****/
  fprintf(fout, "double gettimeoverhead() {\n");
  fprintf(fout, "  TIMESTRUCT start, finish;\n");
  fprintf(fout, "  double result, min = 1000000, avg, max = 0;\n");
  fprintf(fout, "  int i;\n");
  fprintf(fout, "  int n = 1000;\n\n");
  fprintf(fout, "  avg = 0;\n");
  fprintf(fout, "  for (i = 0; i < n; i++) {\n");
  fprintf(fout, "    CLOCK(start);\n");
  fprintf(fout, "    CLOCK(finish);\n");
  fprintf(fout, "    TIMEDIFF(start, finish, result);\n");
  fprintf(fout, "    if (result < min && result >= 0)\n");
  fprintf(fout, "        min = result;\n");
  fprintf(fout, "    if (result >  max)\n");
  fprintf(fout, "        max = result;\n");
  fprintf(fout, "    avg += result;\n");
  fprintf(fout, "  }\n");
  
  if (strncmp(fMode, "MULTICORE", 9) == 0) {
	fprintf(fout, "  printf(\"Proc: %%d, Each call of the time function takes min %%.9f seconds = %%f cycles\\n\",\n");
	fprintf(fout, "           myid, min, min*MHZ*1000000.0);\n");
	fprintf(fout, "  printf(\"Proc: %%d, Each call of the time function takes max %%.9f seconds = %%f cycles\\n\",\n");
	fprintf(fout, "           myid, max, max*MHZ*1000000.0);\n");
	fprintf(fout, "  printf(\"Proc: %%d, Total %%.9f, Average access time is %%.9f seconds for %%d times\\n\\n\",\n");
	fprintf(fout, "           myid, avg, avg/n, n);\n");
  }
  else  {
	fprintf(fout, "  printf(\"Each call of the time function takes min %%.9f seconds = %%f cycles\\n\",\n");
	fprintf(fout, "           min, min*MHZ*1000000.0);\n");
	fprintf(fout, "  printf(\"Each call of the time function takes max %%.9f seconds = %%f cycles\\n\",\n");
	fprintf(fout, "           max, max*MHZ*1000000.0);\n");
	fprintf(fout, "  printf(\"Total %%.9f, Average access time is %%.9f seconds for %%d times\\n\\n\",\n");
	fprintf(fout, "           avg, avg/n, n);\n");
  }
  fprintf(fout, "  return avg/n;\n");
  fprintf(fout, "}\n");

  /***** initIndexArray ******/
  if (strncmp(fPattern, "RANDOM", 6) == 0) {
    fprintf(fout, "int initIndexArray(int length) {\n");
    fprintf(fout, "  int i, j, k;\n");
    if (strncmp(fAlgo, "NESTED", 5) == 0) {
      fprintf(fout, "    for (i = 0; i < length; i++) {\n");
      fprintf(fout, "        ind[i] = getIndex() * L;\n");
      fprintf(fout, "    }\n");
    }
    else {
      fprintf(fout, "    for (i = 0; i < length; i += L) {\n");
      fprintf(fout, "        ind[i] = getIndex() * L;\n");
      fprintf(fout, "        for (j = 1; (j < L) && (i+j < length); j++) { \n");
      fprintf(fout, "          ind[i+j] = ind[i] + j;\n");
      fprintf(fout, "        }\n");
      fprintf(fout, "    }\n");
    }
    fprintf(fout, "  return 0;\n");
    fprintf(fout, "}\n");
  
    /******* getIndex ********/
    fprintf(fout, "long long int getIndex() {\n");
    fprintf(fout, "  return pow(drand48(), 1/K) * (M/L -1);\n");
    fprintf(fout, "}\n");
  }

  /***** Kernel Implementation *****/
  /***** Input: RANDOM/K,M,L,B/, STRIDE/M,S,?/ *****/
  fprintf(fout, "double test() {\n");
  fprintf(fout, "  int i, j, k, v, m, times, length;\n");
  fprintf(fout, "  int extra = 0;\n");
  fprintf(fout, "  long long int jj, kk, pp;\n");
  fprintf(fout, "  double sum = 0.0, t;\n");
  fprintf(fout, "  double c0");
  for (i = 1; i < dReg[0]; i++)
    fprintf(fout, ", c%d", i);
  fprintf(fout, ";\n");
  if (strncmp(fStore, "ARRAY", 5) != 0) {
    fprintf(fout, "  double W0");
    for (i = 1; i < dReg[0]; i++) {
	fprintf(fout, ", W%d", i);
    }
    fprintf(fout, ";\n");

    for (i = 0; i < dReg[0]; i++) {
        fprintf(fout, "  W%d = 0.0;\n", i); 
    }
  }  
  else { 
    for (i = 0; i < dReg[0]; i++) {
       fprintf(fout, "  for (j = 0; j < B; j++) {\n");
       fprintf(fout, "    W%d[j] = 0.0;\n", i);
       fprintf(fout, "  }\n");
    }
  }

  for (i = 0; i < dReg[0]; i++) {
    fprintf(fout, "  c%d = 0.1 + 0.01*%d;\n", i, i);
  }
  fprintf(fout, "  time3 = 0;\n");
  fprintf(fout, "  avg = 0;\n");
  fprintf(fout, "  sum2 = 0;\n");
 
  fprintf(fout, "  times = N;\n");
  fprintf(fout, "  extra = %d;\n", dWarmup);

  if (fRandom) {
    if (nSpatial && (strncmp(fAlgo, "NESTED", 6) == 0) && (N > 100))
        fprintf(fout, "  times = MAX(MIN_N, N/L);\n");
	if (strncmp(fMode, "MULTICORE", 9) != 0) 
		fprintf(fout, "  for (i = 0; i < times + extra; i++) {\n");
    else
		fprintf(fout, "  for (i = 0; i < times + 2*extra; i++) {\n");
	if (strncmp(fAlgo, "FUSED", 5) == 0) {
		fprintf(fout, "    length = B;\n");
        fprintf(fout, "    initIndexArray(length);\n");
		fprintf(fout, "    CLOCK(time1);\n");
        fprintf(fout, "    for (j = 0; j < length; j++) {\n");
        for (i = 0; i < dReg[0]; i++) {
			if (strncmp(fStore, "ARRAY", 5) == 0) {
				fprintf(fout, "      W%d[j] = W%d[j]", i, i);
			}
			else { /* SCALAR */
				fprintf(fout, "      W%d = W%d", i, i);
			}
			for (j = 0; j < dCI[0]; j++) {
				fprintf(fout, "+c%d*(data%d[ind[j]]", i, (i+j)%dReg[0]); 
			}
			for (j = 0; j < dCI[0]; j++) {
				fprintf(fout, ")");
			}
			fprintf(fout, ";\n");
        }
    }
    else { /* NESTED */
        fprintf(fout, "    length = B / L;\n");
        fprintf(fout, "    initIndexArray(length);\n");
		fprintf(fout, "    CLOCK(time1);\n");
        fprintf(fout, "    for (j = 0; j < length; j++) {\n");
		fprintf(fout, "      for (k = 0; k < L; k++) {\n");
        for (i = 0; i < dReg[0]; i++) {
			if (strncmp(fStore, "ARRAY", 5) == 0) {
				fprintf(fout, "        W%d[j*L+k] = W%d[j*L+k]", i, i);
			}
			else { /* SCALAR */
				fprintf(fout, "        W%d = W%d", i, i);
			}
			for (j = 0; j < dCI[0]; j++) {
				fprintf(fout, "+c%d*(data%d[ind[j]+k]", i, (i+j)%dReg[0]);
			}
			for (j = 0; j < dCI[0]; j++) {
				fprintf(fout, ")");
			}
			fprintf(fout, ";\n");
		}
		fprintf(fout, "      }\n");
	}
    fprintf(fout, "    }\n");
	
	if (strncmp(fMode, "MULTICORE", 9) != 0) 	
		fprintf(fout, "    if (i >= extra) { \n");
	else
		fprintf(fout, "    if ((i >= extra) && ( i < times + extra)) { \n");
		
    fprintf(fout, "      CLOCK(time2);\n");
    fprintf(fout, "      TIMEDIFF(time1, time2, time4);\n");
    fprintf(fout, "      time4 -= timeoverhead;\n");
    fprintf(fout, "      time3 += time4;\n");
    fprintf(fout, "      sum2 += time4 * time4;\n");
    fprintf(fout, "    }\n");
    fprintf(fout, "  }\n");

    fprintf(fout, "  t = time3 / times; kavg = t * 1000000; kflops = B * R * CI * 2; \n");
    fprintf(fout, "  flops = (B * R * CI * 2) / t;\n");

    if (strncmp(fStore, "ARRAY", 5) == 0) {
      fprintf(fout, "  kmain = B * R; kdest = 2 * B * R; \n");
      if (strncmp(fAlgo, "NESTED", 6) == 0) {
		fprintf(fout, "  kindex = B / L * R;\n");
        fprintf(fout, "  avg = time3 / (1.0 * times * R * (B+2*B));\n");
        fprintf(fout, "  if (times > 1) { \n");
        fprintf(fout, "    sum2 = sqrt((sum2 - times*t*t)/(times-1))/(1.0*R*(B+2*B));\n");
      }
      else { /* FUSED */
		fprintf(fout, "  kindex = B * R;\n");
        fprintf(fout, "  avg = time3 / (1.0 * times * B * R * 3);\n");
        fprintf(fout, "  if (times > 1) { \n");
        fprintf(fout, "    sum2 = sqrt((sum2 - times*t*t)/(times-1))/(1.0*B * R * 3);\n");
      }
      fprintf(fout, "  }\n");
      fprintf(fout, "  sum = 0;\n");
      for (i = 0; i < dReg[0]; i++) { 
        fprintf(fout, "  for (j = 0; j < B; j++) \n");
        fprintf(fout, "      sum += W%d[j]; \n", i);
      }

    }
    else { /* SCALAR */
      fprintf(fout, "  kmain = B * R; kdest = 0; \n");
      if (strncmp(fAlgo, "NESTED", 6) == 0) {
        fprintf(fout, "  kindex = B / L * R;\n");
        fprintf(fout, "  avg = time3 / (1.0 * times * R * (B));\n");
        fprintf(fout, "  if (times > 1) { \n");
        fprintf(fout, "    sum2 = sqrt((sum2 - times*t*t)/(times-1))/(1.0*R*(B));\n");
      }
      else { /* FUSED */
        fprintf(fout, "  kindex = B * R;\n");
        fprintf(fout, "  avg = time3 / (1.0 * times * B * R * 2);\n");
        fprintf(fout, "  if (times > 1) { \n");
        fprintf(fout, "    sum2 = sqrt((sum2 - times*t*t)/(times-1))/(1.0*B * R * 2);\n");
      }
      fprintf(fout, "  }\n");

      fprintf(fout, "  sum = W0");
      for (j = 1; j < dReg[0]; j++)
        fprintf(fout, "+W%d", j);
      fprintf(fout, ";\n");
    }
  }
  fprintf(fout, "  return sum;\n");
  fprintf(fout, "}\n");
}

void checkSanity() {
  int err = 0, i, tt;
  long long int msize = dMemSize[0] / dReg[0];
  long long int t;

  if (strncmp(fPattern, "RANDOM", 5) == 0) {
    if (nSpatial) {
      t = dSpatial[nSpatial-1];
      if (t == 0) t = dMemSize[0];
    }
    else {
      t = dMaxCurLoads[nMaxCurLoads-1];
    }
	
    tt = dMemSize[0] / t / dReg[0];
    if (tt < 1) {
      printf(" M (%lld) is too small for tt = %d and R = %d\n", dMemSize[0], tt, dReg[0]);
      err = 1;
    }

    if (nSpatial == 0) {
      for (i = 0; i < nMaxCurLoads; i++) {
        if (dMaxCurLoads[i] > MAXNCL) {
			printf(" B (%lld) should not exceed maximum value %d\n", dMaxCurLoads[i], MAXNCL);
			err = 1;
        }
        if (dMaxCurLoads[i] > msize) {
          printf(" B (%lld) should not exceed mem size %d\n", dMaxCurLoads[i], msize);
          err = 1;
        }
      }
      /* B should be multiple of L ? */
    }
    else {
      for (i = 0; i < nSpatial; i++) {
        if (dSpatial[i] > msize) {
          printf(" L (%d) should not exceed maximum value %d\n", dSpatial[i], msize);
          err = 1;
        }
      }
    }
  }
  if (err)  {
	printf("!!!! BENCHMARK PROGRAM WILL NOT BE GENERATED DUE TO ABOVE PROBLEMS!!!!\n");
	exit(1);
  }
}

int main(int argc, char *argv[]) {

  int sel;

  fin = fopen("input", "r");
  if (fin == NULL) {
    printf("can not find input file for parameter setting!");
    exit(1);
  }
  
  /* set default values */
  nReg = 1;
  dReg[0] = 1;
  nCI = 1;
  dCI[0] = 1;

  /* Please do not change the first symbol of each line of input file*/

  retc = fgets(buffer, 1024, fin);
  while ((retc != NULL) && (strlen(buffer) > 1)) {
    sel = findSel(buffer);
    retc = fgets(buffer, 1024, fin);
  }
  fclose(fin);
  
  printInfo();
  checkSanity();

  if ((strncmp(fMode, "PARALLEL", 8) == 0) && (strncmp(fParLang, "CAF", 3) == 0)) 
    createFFile();
  else
    createCFile();

}


