spectre:
	gcc -march=armv8-a -g -o spectre -O0 victimCode1bit.s spectre.c cacheFlush3.s

spectreF:
	gcc -march=armv8-a -g -o spectreF -O0 victimCode1bit.s spectreF.c cacheFlush3.s

cacheFlush2:
	gcc -march=armv8-a -g -o cacheFlushTest2 -O0 cacheFlush2.s cacheFlushTest2.c cacheFlush3.s

cacheFlush:
	gcc -march=armv8-a -g -c cacheFlush.s
	gcc -march=armv8-a -g -c cacheFlushTest.c
	gcc -march=armv8-a -g -o cacheFlushTest cacheFlush.o cacheFlushTest.o

timeAnaly:
	gcc -g -o timestats cacheTimeAnalysis.c

spectre2:
	gcc -march=armv8-a -g -o spectre2 -O0 cacheFlush2.s spectre2.c cacheFlush3.s 

provaFlush:
	gcc -march=armv8-a -g -o pFlush cacheFlush3.s provaFlush.c

provaRandomAccess:
	gcc -march=armv8-a -g -o pRandAxs cacheFlush3.s provaRandomAccess.c

spectre1bit:
	gcc -march=armv8-a -g -o spectre1bit cacheFlush2.s spectre1bit.c cacheFlush3.s 

spectreRndAxs:
	gcc -march=armv8-a -g -o spectreRndAxs -O0 cacheFlush2.s spectreRndAxs.c cacheFlush3.s 

provarand:
	gcc -march=armv8-a -g -o prand cacheFlush3.s provaRArand.c

spectre1bFull:
	gcc -march=armv8-a -g -o spectre1bF -O0 victimCode1bit.s spectre1bitFull.c cacheFlush3.s 

spectreRA2:
	gcc -march=armv8-a -g -o spectreRndAxs2 -O0 cacheFlush2.s spectreRndAxs2.c cacheFlush3.s 

FF:
	gcc -march=armv8-a -g -o FF -O0 FF.c FlushFF.s 

spectrera3:
	gcc -march=armv8-a -g -o spectreRndAxs3 -O0 cacheFlush2.s spectreRndAxs3.c cacheFlush3.s 

spectrera4:
	gcc -march=armv8-a -g -o spectreRndAxs4 -O0 cacheFlush2.s spectreRndAxs4.c cacheFlush3.s 

spectrera5:
	gcc -march=armv8-a -g -o spectreRndAxs5 -O0 cacheFlush2.s spectreRndAxs5.c cacheFlush3.s 

spectrera6:
	gcc -march=armv8-a -g -o spectreRndAxs6 -O0 cacheFlush2.s spectreRndAxs6.c cacheFlush3.s 

spectrera7:
	gcc -march=armv8-a -g -o spectreRndAxs7 -O0 cacheFlush2.s spectreRndAxs7.c cacheFlush3.s 

spectrera8:
	gcc -march=armv8-a -g -o spectreRndAxs8 -O0 cacheFlush2.s spectreRndAxs8.c cacheFlush3.s 

spectrera9:
	gcc -march=armv8-a -g -o spectreRndAxs9 -O0 cacheFlush2.s spectreRndAxs9.c cacheFlush3.s 

spectrera10:
	gcc -march=armv8-a -g -o spectreRndAxs10 -O0 cacheFlush2.s spectreRndAxs10.c cacheFlush3.s 

spectrera11:
	gcc -march=armv8-a -g -o spectreRndAxs11 -O0 cacheFlush2.s spectreRndAxs11.c cacheFlush3.s 
