	.section    .text,"ax",@progbits
	.global DHandler
	.global DTrapUnhandled
	.global DTrapDisk 
		
DHandler   GET 	$0,rQ
	   SUBU	$1,$0,1		%from xxx...xxx1000 to xxx...xxx0111
	   SADD	$2,$1,$0	%position of lowest bit
	   ANDN	$1,$0,$1	%the lowest bit
           ANDN	$1,$0,$1	%delete lowest bit
	   PUT	rQ,$1		%and return to rQ
	   SLU	$2,$2,2		%scale
           GETA	$1,DTrapTable	%and jump
	   GO	$1,$1,$2


DTrapUnhandled	SWYM	5               % inform the debugger
		POP	0,0

DTrapDisk       POP	0,0		% ignore
