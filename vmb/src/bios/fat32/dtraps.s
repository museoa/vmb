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

	%	Required dynamic Traps
	.global DTrapReboot
DTrapReboot		JMP	Boot
	
	.global DTrapMemParityError
DTrapMemParityError	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Memory parity error",0

	.global DTrapMemNonExiistent
DTrapMemNonExiistent	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Access to nonexistent Memory",0

	.global DTrapPowerFail
DTrapPowerFail	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Power Fail - switching to battery ;-)",0

	.global DTrapPageTableError
DTrapPageTableError	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Error in page table structure",0

	.global DTrapPageFault
DTrapPageFault		GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Page fault - not yet implemented",0

	.global DTrapIntervall
DTrapIntervall		GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Intervall Counter rC is zero",0


	.global DTrapPrivileged
DTrapPrivileged		GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Privileged Instruction",0

	.global DTrapSecurity
DTrapSecurity	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Security violation",0

	.global DTrapRuleBreak
DTrapRuleBreak	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Illegal Instruction",0

	.global DTrapKernelOnly
DTrapKernelOnly	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Instruction for kernel use only",0

	.global DTrapTanslationBypass
DTrapTanslationBypass	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Illegal access to negative address",0

	.global DTrapNoExec
DTrapNoExec	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Missing execute permission",0

	.global DTrapNoWrite
DTrapNoWrite	GETA	$255,1F
			SWYM	255,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG  Missing write permission",0

	.global DTrapNoRead
DTrapNoRead	GETA	$255,1F
		SWYM	255,5		% tell the debugger
		POP	0,0
1H		BYTE    "DEBUG Missing read permission",0


	.global	DTrapUnhandled	
DTrapUnhandled	GETA	$255,1F
		SWYM	255,5		% tell the debugger
		POP	0,0
1H		BYTE    "DEBUG Trap not implemented",0


	.global DTrapIgnored
DTrapIgnored	POP	0,0		% ignore

	.global DTrapDisk
DTrapDisk       POP	0,0		% ignore


