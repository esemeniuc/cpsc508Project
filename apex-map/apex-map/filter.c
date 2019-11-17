#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
double results[1024];
int num = 0;
FILE *fin;
char str[100];
char buf[1024];
double MHZ;
char info[40];
int fformat;

double MEM;
int REG, COMP, INDEX;
char LOOP[20], STORE[20];
int ADATA, AINDEX, ADEST;
int FLOPS;
double TIME, K;
int L;
 
float getnum(char *p) {
   double time;
   int flops, dall, dmain, dindex, ddest;
   char *p1, *p2;

   while (*p < '0' || *p > '9') p++;
   p1 = p;
   while (*p != ',') p++;
   time = atof(p1);

   while (*p < '0' || *p > '9') p++;
   p1 = p;
   while (*p != ',') p++;
   flops = atoi(p1);

   while (*p < '0' || *p > '9') p++;
   p1 = p;
   while (*p != ',') p++;
   dmain = atoi(p1);

   while (*p < '0' || *p > '9') p++;
   p1 = p;
   while (*p != ',') p++;
   dindex = atoi(p1);

   while (*p < '0' || *p > '9') p++;
   p1 = p;
   while (*p != '\n') p++;
   ddest = atoi(p1);

   if (info[0] == 'M') return flops/time;
   if (info[0] == 'B') {
        if (info[1] == 'A') 
          return (dmain * sizeof(double)+dindex * sizeof(long long int)+ddest * sizeof(double))/time;
        if (info[1] == 'P')  
          return (dmain * sizeof(double)+dindex * sizeof(long long int))/time;
        if (info[1] == 'M')  
          return (dmain * sizeof(double))/time;
   }
   if (info[0] == 'C') {
        if (info[1] == 'A')  
          return time /(dmain+dindex+ddest) * MHZ;
        if (info[1] == 'P')  
          return time /(dmain+dindex) * MHZ; 
        if (info[1] == 'M')  
          return time /(dmain) * MHZ; 
   }
   if (info[0] == 'T') {
        if (info[1] == 'A')
          return time * 1000 /(dmain+dindex+ddest);
        if (info[1] == 'P')
          return time * 1000 /(dmain+dindex);
        if (info[1] == 'M')
          return time * 1000 /(dmain);
   }

}
 
void usage() {

 /* the format will decide the data file format */
 /* Based on the file format, you can extract different info */
 /* ------------------------------------------- */
 /* format == 1 : old data format */
 /* info = MF (Mflops) */
 /*        BA (bandwidth for all data access) */
 /*        BP (bandwidth for main data access and index */
 /*        BM (bandwidth for main data access) */
 /*        CA (cycles/data access for all data access) */
 /*        CP (cycles/data access for main data and index */
 /*        CM (cycles/data access for main data access) */
 /*        TA (ns/data access for all data access) */
 /*        TP (ns/data access for main data and index */
 /*        TM (ns/data access for main data access) */
 /* -------------------------------------------- */

 printf("Usage: a.out DataFile FileFormat Info\n");
 printf(" FileFormat = 1, Info = {MF, BA, BP, BM, CM, CP, CA, TA, TP, TM}\n");
 printf(" See comments in uasge() function for details.\n");
}

int main(int argc, char *argv[]) {
 
 int i, j;
 char *c;
 
 if (argc < 3) {
   usage();
   exit(1);
 }
 
 fin = fopen(argv[1], "r");
 if (fin == NULL) {
   printf("can not open file: %s\n", argv[1]);
   exit(1);
 }
 
 fformat = atoi(argv[2]);
 if ((fformat != 1) &&  (fformat != 2)) {
   printf("File format %d is not defined!\n", fformat);
   exit(1);
 }

 if (argc > 3) strcpy(info, argv[3]);

/**
 while (fgets(buf, 1024, fin) != NULL) {
   if ((c = strstr(buf, "CPU Frequency : ")) != NULL) {
        c += 16;
        MHZ = atof(c);
	continue;
   }
   if ((c = strstr(buf, "Memory Size is")) != NULL) {
	c += 15;
	MEM = atof(c);
        c = strstr(buf, "Register Pressure:");
	c += 19;
        REG = atoi(c);
	continue;
    }
    if ((c = strstr(buf, "Computational Intensity:")) != NULL) {
	c += 24;
	COMP = atoi(c);
	continue;
    }
    
    if ((c = strstr(buf, "Computational Kernel")) != NULL) {
	c += 22; i = 0;
	while (*c != ' ') LOOP[i++] = *c++;
        LOOP[i] = '\0';
        c = strstr(buf, "Implementation");
	c += 16; i = 0;
	while (*c != ' ' && *c != '\n') STORE[i++] = *c++;
        STORE[i] = '\0';
        c = strstr(buf, "Index Length");
        c += 14; i = 0;
        INDEX = atoi(c);

        break;
    }
 }
**/
/*
 printf("MHZ %.0f, MEM %.0f, COMP %d, REG %d, LOOP %s, STORE %s, INDEX %d\n", MHZ, MEM, COMP, REG, LOOP, STORE, INDEX);
*/
 /* out specified info */
 if (fformat == 1) {
     while (fgets(buf, 1024, fin) != NULL) {
         if ((c = strstr(buf, "destination")) != NULL) {
             results[num++] = getnum(c);
         }
     }
     fclose(fin);
     for (i = 0; i < num; i++) {
        if (i % 10 == 0)
            printf("\n");
        printf("%.2f ", results[i]);
     }
     printf("\n");
 }
 
 /* out for excel format */
 if (fformat == 2) {
    while (fgets(buf, 1024, fin) != NULL) {
 	/** moved here for cases with multi runs data in one file */
	if ((c = strstr(buf, "CPU Frequency : ")) != NULL) {
        	c += 16;
        	MHZ = atof(c);
        	continue;
   	}
	if ((c = strstr(buf, "Memory Size is")) != NULL) {
        	c += 15;
        	MEM = atof(c);
        	c = strstr(buf, "Register Pressure:");
        	c += 19;
        	REG = atoi(c);
        	continue;
    	}
        if ((c = strstr(buf, "Computational Intensity:")) != NULL) {
        	c += 24;
        	COMP = atoi(c);
        	continue;
        }
  
        if ((c = strstr(buf, "Computational Kernel")) != NULL) {
        	c += 22; i = 0;
        	while (*c != ' ') LOOP[i++] = *c++;
        	LOOP[i] = '\0';
        	c = strstr(buf, "Implementation");
        	c += 16; i = 0;
        	while (*c != ' ' && *c != '\n') STORE[i++] = *c++;
        	STORE[i] = '\0';
        	c = strstr(buf, "Index Length");
        	c += 14; i = 0;
        	INDEX = atoi(c);
		continue;
        }

	if ((c = strstr(buf, "K=")) != NULL) {
	    c += 2;
	    K = atof(c);
	    c += 10;
	    L = atoi(c);
	    /* assume these two lines next to each other, otherwise need change here */
	    fgets(buf, 1024, fin);
	    c = strstr(buf, "destination");
	    if (c == NULL) {
		printf("the outpu format is not right!\n");
		exit(1);
	    }
	    c += 12;
	    TIME = atof(c);
 	    while (*c != ',') c++;
	    c += 2;
	    FLOPS = atoi(c);
	    while (*c != ',') c++;
	    c += 2;
	    ADATA = atoi(c);
	    while (*c != ',') c++;
	    c += 2;
	    AINDEX = atoi(c);
	    while (*c != ',') c++;
	    c += 2;
	    ADEST = atoi(c);
	    printf("%.0f %d %.3f %d %d %d %s %s %.3f %d %d %d %d\n", MEM, L, K, INDEX, REG, COMP, LOOP, STORE, TIME, FLOPS, ADATA, AINDEX, ADEST);   
	}
    }
    fclose(fin);
 }

}

