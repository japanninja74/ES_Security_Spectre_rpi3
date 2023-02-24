.globl Flush
.text

//.align 2

.type Flush, %function
Flush:
	DSB ISH
	ISB
	DC CIVAC, X0
	DSB ISH
	ISB
	RET
