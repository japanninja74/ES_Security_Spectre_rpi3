#pragma GCC push_options
#pragma GCC optimize ("O0")
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include "armpmu_lib.h"
#include <unistd.h>

/* 	The prefetch algorithm fetch every page containing array2 as soon as it detects that we are making numerous access to the array.
	This makes impossible to read more than one page at once (I've tried almost every variation of the technique I'm using).
	The lines of every page are accessed in "random" order since accessing them in order cause some noise.
	This noise is more complex to even out (since we have to perform more than one cycle) than it is to shuffle the accesses vector

	PROBLEM: A line cached in one page will be cached also in the others when using this approach --> Raising SAME_READ_CYCLE too much and VOTE>1 won't help
			 --> Rotating the coefficients of the shuffle function does not work
			 --> Randomizing the index of base and v at each SAME_READ_CYCLE cycle doesn't change this behaviour
			 --> Randomizing the index of base and v at each page read for every SAME_READ_CYCLE cycle doesn't change this behaviour
			 --> Reading just that line one page at a time in any order (even randomized) will not help after the 57th byte
			 --> Considering correct the first line with the lowest access within a threshold returns wrong bytes after 120 bytes read
				 Can't rely anymore on the fact that the line in the page before the one accessed is not cached
			 --> Reading half page at a time yiedls worse results
			 --> The problem starts at line 122 since reading starting from a random byte don't change this fact
				 From line 122 even the line in the page before the one with the line actually accessed starts to be cached
			 --> When a line is accessed above the line 121 (starting the count from 0) that line is prefetched in the other pages starting
			 	 from the bottom (I assume the assumption behind this behaviour is that we are going to access the array going forward)
				 From line 121 to 127 the prefetch starts from the top (I think assuming we are going backwards in the array).
				 From 128 to 256 the prefetch starts again from the bottom (At this point I have no clue why this is happening --> try accessing the pages from bottom to top).
				 INVESTIGATE FURTHER ON THIS SINCE CHANGING THE SIDE OF ACCESS WITHIN THE PAGES CHANGES SOME TIMINGS (CHECK VARIOUS COMBINATION ACCESSING PAGES FROM 3 TO 0 TOO)

				- Pages [0 to 3] Lines [0 to 64] : 	Lines from 0 to 120 	pages cached from bottom 	(page 0 high latency)
													Lines from 121 to 127 	pages cached from top		(page 3 high latency)
													Lines from 128 to 184 	pages cached from bottom	(page 0 high latency) 
													Lines from 185 to 191	pages cached from top		(page 3 high latency)
													Lines from 186 to 256   pages cached from bottom	(page 0 high latency)
				- Pages [3 to 0] Lines [0 to 64] : 	Same as Pages [0 to 3]
				- Pages [0 to 3] Lines [63 to 0] :	Same as previous
				- Pages [3 to 0] Lines [0 to 63] :  Same as previous

			--> Changing the coefficients of the shuffle function the prefetching behaviour changes as well --> Too complex to decode
*/

#define OOB_READ 256			// Number of byte read
#define ARRAY_SIZE 8			// Array1 size
//#define MISTRAIN 3				// Parameter for mistraining
#define CACHE_LINES 128			// Lines of L1 cache
#define ARRAY2_SIZE 256*64
#define SAME_READ_CYCLES 5
#define SHFL_DIM_LINE 64		// Dimension of the shuffle pool
#define TMIN 40           		// Minimum cache read latency not considered as hit
#define LOWLAT_LINES 10
#define PAGES 4

//extern void victimCode(int* array1_size,char* array1,char* array2,int *junk,int x);
extern long int Flush(char* line);
void AccessShuffle(int*letture,int base,int v,int mod);

// Dichiarate globali su suggerimento di Davide (nessun motivo specifico)
// L1 CACHE LINE SIZE: 64 byte
// VALUES A BYTE CAN ASSUME: 256

// Shuffle Accesses
int v[SAME_READ_CYCLES]={229, 227, 223, 211, 199, 197, 193, 191, 181, 179/*, 173, 167, 163, 157, 151, 149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73, 71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, /*11, 7, 5, 3, 2*/};
int base[SAME_READ_CYCLES]={/*73, 79, 83, 89, 97,*/ 101, 103, 107, 109, 113, /*127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263,*/ 269, 271, 277, 281, 283,/* 293, 307, 311, 313, 317, 331, 337, 347, 349*/};
int line[SHFL_DIM_LINE];

// Victim variables
char array1[]="KnownStrSecretStr";
char array2[ARRAY2_SIZE];
int array1_size = ARRAY_SIZE;
char *addr;

// Attacker variables
int x;

// Variable for byte read from cache side channel
uint32_t start,end;
uint32_t timings[256];
int junk;
int page_order[PAGES]={1,2,0,3};
char secret_byte;
// Just indexes
int i,j,k,h,m,l,d;

// Cache timings file
FILE* fout;

// Read stats
int below=0;

int main(void){

	// Timing File
	if((fout=fopen("cacheTimings.txt","w"))==NULL){
		printf("File open error\n");
		exit(1);
	}

	// Byte read
	for(i=0,x=0;i<OOB_READ;i++,x++){

		// Timing array clean
		for(j=0;j<256;j++)
			timings[j]=0;

		// SAME_BYTE_CYCLES tries for each byte
  		for(j=0;j<SAME_READ_CYCLES;j++){

            AccessShuffle(line,base[j],v[j],SHFL_DIM_LINE);

			for(l=0;l<PAGES;l++){
				// Flush
				for(k=0;k<ARRAY2_SIZE;k++)
					Flush(array2+k);

				for(k=0;k<ARRAY_SIZE;k++)
					Flush(array1+k);

				for(k=0;k<256;k++)
					Flush(timings+k);

				for(k=0;k<SAME_READ_CYCLES;k++){
					Flush(base+k);
					Flush(v+k);
				}

				Flush(&array1_size);

	            junk = array2[x*64];

	            for(k=0;k<64;k++){
					// Non-cached read
					addr = &array2[l*4096+line[k]*64];
		            start=rdtsc32();
		            junk &= *addr;
		            end=rdtsc32();
		            timings[line[k]+l*64]+=end-start;
					//printf("Page %d, Line %d, Timing %u\n",l,line[k],end-start);
				}

			} // --> Page range
		}

  		// Avg timing for each cache line read
		for(k=0;k<256;k++){
	  	  	timings[k]/=SAME_READ_CYCLES;
	  		fprintf(fout,"%u ",timings[k]);

			// Numbers below TMIN detect --> trying to find a threshold
            if(timings[k]<TMIN){
	            below++;
//            	printf("Run #%d",i);
//            	printf("--> %u\n",timings[k]);

			}
	  	}

		for(k=0;k<PAGES;k++){
			junk=64*k+x%64;
			printf("Byte %d --> Line : %d,  Latency: %u\n",x,junk,timings[junk]);
			/*if(timings[junk]<(min_latency+TOLERANCE)){
				printf("Byte %d --> Value read: %d\n",i,junk);
			}*/
		}

		fprintf(fout,"\n");
	}
	printf("Below threshold (%d): %d\n",TMIN,below);
	fclose(fout);
  	return 0;
}


void AccessShuffle(int* letture,int base,int v,int mod){ // Algorithm similar to hashing function for strings
	for (int i=0; i<mod; i++){
		letture[i] = ((i * base) + v) % mod;
	}
	return;
}

#pragma GCC pop_options
