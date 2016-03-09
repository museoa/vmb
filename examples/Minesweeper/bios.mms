%	this is the MMIX BIOS
%	it is considert to be the ROM loaded 
%	at physical address 0000 0000 0000 0000
%	used with 
%	virtual address 8000 0000 0000 0000

	.section    .text,"ax",@progbits		
	LOC	#8000000000000000
	
% page table setup (see small model in address.howto)
Main	IS	@  dummy	%Main, to keep mmixal happy
Boot	GETA	$0,DTrap	%set dynamic- and forced-trap  handler
	PUT	rTT,$0
	GETA	$0,FTrap
	PUT	rT,$0
	PUSHJ	$0,memory	%initialize the memory setup
	GET	$0,rQ
        SET	$0,0
	PUT     rQ,$0		%clear interrupts

%	here we start a loaded user program
%       rXX should be #FB0000FF = UNSAVE $255
%	rBB is coppied to $255, it should be the place in the stack 
%	where UNSAVE will find its data
%	rWW should be the entry point in the main program, 
%	thats where the program
%	continues after the UNSAVE.
%	If no program is loaded, rXX will be 0, that is TRAP 0,Halt,0
%	and we end the program before it has started in the Trap handler.
	
	NEG	$255,1	% enable interrupt $255->rK with resume 1
	RESUME	1	% loading a file sets up special registers for that

	
DTrap	PUSHJ	$255,DHandler
	PUT	rJ,$255
	NEG	$255,1		% enable interrupt $255->rK with resume 1
	RESUME	1
	
DHandler 	GET 	$0,rQ
		SUBU	$1,$0,1		%from xxx...xxx1000 to xxx...xxx0111
		SADD	$2,$1,$0	%position of lowest bit
		ANDN	$1,$0,$1	%the lowest bit
        	ANDN	$1,$0,$1	%delete lowest bit
		PUT	rQ,$1		%and return to rQ
		SLU	$2,$2,2		%scale
        	GETA	$1,DTrapTable	%and jump
		GO	$1,$1,$2

DTrapTable JMP DTrapReboot			%0	the machine bits
 	    JMP DTrapMemParityError		%1
           JMP DTrapMemNonExiistent	%2
           JMP DTrapPowerFail		%3
           JMP DTrapPageTableError		%4
           JMP DTrapUnhandled		%5
           JMP DTrapUnhandled		%6
           JMP DTrapIntervall		%7

           JMP DTrapUnhandled  %8
           JMP DTrapUnhandled  %9
           JMP DTrapUnhandled  %10
           JMP DTrapUnhandled  %11
           JMP DTrapUnhandled  %12
           JMP DTrapUnhandled  %13
           JMP DTrapUnhandled  %14
           JMP DTrapUnhandled  %15

           JMP DTrapUnhandled  %16
           JMP DTrapKey        %17
           JMP DTrapScreen     %18
           JMP DTrapMouse      %19
           JMP DTrapGPU	   %20
           JMP DTrapTimer      %21
           JMP DTrapDisk       %22
           JMP DTrapUnhandled  %23
           JMP DTrapUnhandled  %24
           JMP DTrapUnhandled  %25
           JMP DTrapUnhandled  %26
           JMP DTrapUnhandled  %27
           JMP DTrapUnhandled  %28
           JMP DTrapUnhandled  %29
           JMP DTrapUnhandled  %30
           JMP DTrapUnhandled  %31

           JMP DTrapPrivileged		%32	% Program bits
           JMP DTrapSecurity		%33
           JMP DTrapRuleBreak		%34
           JMP DTrapKernelOnly		%35
           JMP DTrapTanslationBypass	%36
           JMP DTrapNoExec		%37
           JMP DTrapNoWrite		%38
           JMP DTrapNoRead		%39

           JMP DTrapUnhandled  %40
           JMP DTrapUnhandled  %41
           JMP DTrapUnhandled  %42
           JMP DTrapUnhandled  %43
           JMP DTrapUnhandled  %44
           JMP DTrapUnhandled  %45
           JMP DTrapUnhandled  %46
           JMP DTrapUnhandled  %47

           JMP DTrapButton0    %48
           JMP DTrapButton1    %49
           JMP DTrapButton2    %50
           JMP DTrapButton4    %51
           JMP DTrapUnhandled  %52
           JMP DTrapUnhandled  %53
           JMP DTrapUnhandled  %54
           JMP DTrapUnhandled  %55
           JMP DTrapUnhandled  %56
           JMP DTrapUnhandled  %57
           JMP DTrapUnhandled  %58
           JMP DTrapUnhandled  %59
           JMP DTrapUnhandled  %60
           JMP DTrapUnhandled  %61
           JMP DTrapUnhandled  %62
           JMP DTrapUnhandled  %63
           JMP DTrapIgnore     %64  rQ was zero

%	Default dynamic Trap Handlers

DTrapUnhandled		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Trap unhandled",0



DTrapIgnore		POP	0,0


%	Required dynamic Trap Handlers

DTrapReboot		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			JMP	Boot
1H			BYTE    "DEBUG Rebooting",0


DTrapMemParityError	GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Memory parity error",0


DTrapMemNonExiistent	GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Access to nonexistent Memory",0


DTrapPowerFail		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Power Fail - switching to battery ;-)",0


DTrapPageTableError	GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Error in page table structure",0


DTrapIntervall		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Intervall Counter rC is zero",0



DTrapPrivileged		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Privileged Instruction",0


DTrapSecurity		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Security violation",0


DTrapRuleBreak		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Illegal Instruction",0


DTrapKernelOnly		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Instruction for kernel use only",0


DTrapTanslationBypass	GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Illegal access to negative address",0


DTrapNoExec		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Missing execute permission",0


DTrapNoWrite		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG  Missing write permission",0


DTrapNoRead		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Missing read permission",0


%	Device specific dynamic Trap Handlers



%	This Bios handles the following devices
%	ROM		at 0x8000 0000 0000 0000
%	RAM		at 0x8000 0001 0000 0000
%	keyboard	at 0x8001 0000 0000 0000
%	mouse		at 0x8001 0000 0000 0010
%	GPU		at 0x8001 0000 0000 0020
%	timer		at 0x8001 0000 0000 0060
%	sevensegment	at 0x8001 0000 0000 0080
%	Video Ram	at 0x8002 0000 0000 0000

IOBase   IS	#8001             %   hi wyde of IOBase	
keyboard IS	#00
screen	 IS	#08
mouse	 IS	#10
GPU	 IS	#20
timer	 IS	#60
sevenseg IS	#80
VRamBase IS	#8002

%	echo a character from the keyboard
DTrapKey SETH	$1,IOBase    			
	LDO	$2,$1,keyboard	keyboard status/data
	BN	$2,1F	
	SR	$3,$2,32
	AND	$3,$3,#FF
	BZ	$3,1F	
	GET	$3,rJ
	AND	$5,$2,#FF
	PUSHJ	$4,ScreenC
	PUT	rJ,$3
1H	POP	0,0

DTrapScreen     IS DTrapIgnore   


%	with an unhandled mouse interrupt we dont do nothing
DTrapMouse	GETA	$0,1F
		SWYM	$0,5               % inform the debugger
		POP	0,0
1H		BYTE	"DEBUG unexpected mouse interrupt ignored",0

DTrapGPU	IS DTrapIgnore
DTrapTimer      IS DTrapIgnore
DTrapDisk       IS DTrapIgnore 

DTrapButton0    IS DTrapIgnore 
DTrapButton1    IS DTrapIgnore
DTrapButton2    IS DTrapIgnore
DTrapButton4    IS DTrapIgnore


%	two auxiliar functions to read and write characters.

%	read blocking a character from the keyboard
KeyboardC 	SETH	$1,IOBase    
3H	LDO	$2,$1,keyboard	%keyboard status/data
	BN	$2,2F		%error indicator
	SR	$3,$2,32
	AND	$3,$3,#FF
	BZ	$3,2F		%no char available
	AND	$0,$2,#FF	%got char in $0
	CMP	$3,$0,#0D
	CSZ	$0,$3,#0A	replace cr by nl
	GET	$3,rJ
	SET	$5,$0
	PUSHJ	$4,ScreenC	%echo
	PUT	rJ,$3
	POP	1,0
	
%	wait
2H	SYNC	4		%go to power save mode
	GET 	$2,rQ
	SET	$3,1
	SL	$3,$3,17	the keyboard interrupt bit
	AND	$4,$2,$3	test it
	BZ	$4,2B           continue waiting
	ANDN	$2,$2,$3	reset the keybaord interrupt bit
	PUT	rQ,$2		and store back to rQ
	JMP	3B

%	Put one character contained in $0 on the screen

%	version for the winvram device with gpu
ScreenC	SETH	$1,IOBase
        SETML	$2,#0100		command in top 8 bit
        OR	$3,$2,$0		char in lo 8 bit
	STT	$3,$1,GPU
	CMP	$3,$0,#0D		CR
	BNZ	$3,1F
	OR	$3,$2,#0A
       STT	$3,$1,GPU
1H	POP	0,0
	
 
%	Entry point for a forced TRAP
FTrap	PUSHJ	$255,FHandler
	PUT	rJ,$255
	NEG	$255,1	  %enable interrupt $255->rK with resume 1
	RESUME	1

FHandler	GET	$0,rXX
		BNN	$0,Ropcode
		SRU	$1,$0,24       
		AND	$1,$1,#FF         
		BNZ	$1,TrapUnhandled  %not a TRAP and ropcode<0	
		SRU	$1,$0,8
		AND	$1,$1,#FF	%the Y value (the function code)
		SL	$1,$1,2
		GETA	$2,FTrapTable
		GO	$2,$2,$1		%Jump into the Trap Table

Ropcode		SRU	$0,$0,56	%the ropcode
		BZ	$0,FTrapPageFault %0: page fault
		SUB	$1,$0,2         
		BZ      $1,Emulate	%2: emulate the instruction
		CMP	$1,$0,3	        
		BZ	$1,Virtual	%3: page table translation
		POP     0,0             %ignore the rest

%       Emulate the instruction
Emulate POP     0,0		%can't happen

%	Do pagetable translation in software
Virtual POP     0,0		%can't happen

FTrapTable JMP   TrapHalt       %0
	   JMP   TrapFopen      %1
	   JMP   TrapFclose     %2
	   JMP   TrapFread      %3
	   JMP   TrapFgets      %4
	   JMP   TrapFgetws     %5
	   JMP   TrapFwrite     %6
	   JMP   TrapFputs      %7 
	   JMP   TrapFputws     %8
	   JMP   TrapFseek      %9
	   JMP   TrapFtell      %a
	   JMP   TrapUnhandled  %b
	   JMP   TrapSSet       %c
	   JMP   TrapTSleep     %d
	   JMP   TrapTDate      %e
	   JMP   TrapTTimeOfDay %f
	   JMP   TrapGPutPixel  %10
	   JMP	 TrapMWaitEvent %11
	   JMP   TrapGPutChar   %12
	   JMP   TrapGPutStr    %13
	   JMP   TrapGLine      %14
	   JMP   TrapGRectangle %15
	   JMP   TrapGBitBlt    %16
	   JMP   TrapGSetPos    %17
	   JMP   TrapGSetTextColor  %18
	   JMP   TrapGSetFillColor  %19
	   JMP   TrapGSetLineColor  %1a
	   JMP   TrapGBitBltIn  %1b
	   JMP   TrapGBitBltOut %1c
	   JMP   TrapUnhandled  %1d
	   JMP   TrapUnhandled  %1e
	   JMP   TrapUnhandled  %1f

	   JMP   TrapUnhandled		%20
	   JMP   TrapUnhandled		%21
	   JMP   TrapUnhandled		%22
	   JMP   TrapUnhandled		%23
	   JMP   TrapUnhandled		%24
	   JMP   TrapUnhandled		%25
	   JMP   TrapUnhandled		%26
	   JMP   TrapUnhandled		%27
	   JMP   TrapUnhandled		%28
	   JMP   TrapUnhandled		%29
	   JMP   TrapUnhandled		%2a
	   JMP   TrapUnhandled		%2b
	   JMP   TrapUnhandled		%2c
	   JMP   TrapUnhandled		%2d
	   JMP   TrapUnhandled		%2e
	   JMP   TrapUnhandled		%2f
	   JMP   TrapUnhandled		%30
	   JMP   TrapUnhandled		%31
	   JMP   TrapUnhandled		%32
	   JMP   TrapUnhandled		%33
	   JMP   TrapUnhandled		%34
	   JMP   TrapUnhandled		%35
	   JMP   TrapUnhandled		%36
	   JMP   TrapUnhandled		%37
	   JMP   TrapUnhandled		%38
	   JMP   TrapUnhandled		%39
	   JMP   TrapUnhandled		%3a
	   JMP   TrapUnhandled		%3b
	   JMP   TrapUnhandled		%3c
	   JMP   TrapUnhandled		%3d
	   JMP   TrapUnhandled		%3e
	   JMP   TrapUnhandled		%3f

	   JMP   TrapUnhandled		%40
	   JMP   TrapUnhandled		%41
	   JMP   TrapUnhandled		%42
	   JMP   TrapUnhandled		%43
	   JMP   TrapUnhandled		%44
	   JMP   TrapUnhandled		%45
	   JMP   TrapUnhandled		%46
	   JMP   TrapUnhandled		%47
	   JMP   TrapUnhandled		%48
	   JMP   TrapUnhandled		%49
	   JMP   TrapUnhandled		%4a
	   JMP   TrapUnhandled		%4b
	   JMP   TrapUnhandled		%4c
	   JMP   TrapUnhandled		%4d
	   JMP   TrapUnhandled		%4e
	   JMP   TrapUnhandled		%4f
	   JMP   TrapUnhandled		%50
	   JMP   TrapUnhandled		%51
	   JMP   TrapUnhandled		%52
	   JMP   TrapUnhandled		%53
	   JMP   TrapUnhandled		%54
	   JMP   TrapUnhandled		%55
	   JMP   TrapUnhandled		%56
	   JMP   TrapUnhandled		%57
	   JMP   TrapUnhandled		%58
	   JMP   TrapUnhandled		%59
	   JMP   TrapUnhandled		%5a
	   JMP   TrapUnhandled		%5b
	   JMP   TrapUnhandled		%5c
	   JMP   TrapUnhandled		%5d
	   JMP   TrapUnhandled		%5e
	   JMP   TrapUnhandled		%5f

	   JMP   TrapUnhandled		%60
	   JMP   TrapUnhandled		%61
	   JMP   TrapUnhandled		%62
	   JMP   TrapUnhandled		%63
	   JMP   TrapUnhandled		%64
	   JMP   TrapUnhandled		%65
	   JMP   TrapUnhandled		%66
	   JMP   TrapUnhandled		%67
	   JMP   TrapUnhandled		%68
	   JMP   TrapUnhandled		%69
	   JMP   TrapUnhandled		%6a
	   JMP   TrapUnhandled		%6b
	   JMP   TrapUnhandled		%6c
	   JMP   TrapUnhandled		%6d
	   JMP   TrapUnhandled		%6e
	   JMP   TrapUnhandled		%6f
	   JMP   TrapUnhandled		%70
	   JMP   TrapUnhandled		%71
	   JMP   TrapUnhandled		%72
	   JMP   TrapUnhandled		%73
	   JMP   TrapUnhandled		%74
	   JMP   TrapUnhandled		%75
	   JMP   TrapUnhandled		%76
	   JMP   TrapUnhandled		%77
	   JMP   TrapUnhandled		%78
	   JMP   TrapUnhandled		%79
	   JMP   TrapUnhandled		%7a
	   JMP   TrapUnhandled		%7b
	   JMP   TrapUnhandled		%7c
	   JMP   TrapUnhandled		%7d
	   JMP   TrapUnhandled		%7e
	   JMP   TrapUnhandled		%7f

	   JMP   TrapUnhandled		%80
	   JMP   TrapUnhandled		%81
	   JMP   TrapUnhandled		%82
	   JMP   TrapUnhandled		%83
	   JMP   TrapUnhandled		%84
	   JMP   TrapUnhandled		%85
	   JMP   TrapUnhandled		%86
	   JMP   TrapUnhandled		%87
	   JMP   TrapUnhandled		%88
	   JMP   TrapUnhandled		%89
	   JMP   TrapUnhandled		%8a
	   JMP   TrapUnhandled		%8b
	   JMP   TrapUnhandled		%8c
	   JMP   TrapUnhandled		%8d
	   JMP   TrapUnhandled		%8e
	   JMP   TrapUnhandled		%8f
	   JMP   TrapUnhandled		%90
	   JMP   TrapUnhandled		%91
	   JMP   TrapUnhandled		%92
	   JMP   TrapUnhandled		%93
	   JMP   TrapUnhandled		%94
	   JMP   TrapUnhandled		%95
	   JMP   TrapUnhandled		%96
	   JMP   TrapUnhandled		%97
	   JMP   TrapUnhandled		%98
	   JMP   TrapUnhandled		%99
	   JMP   TrapUnhandled		%9a
	   JMP   TrapUnhandled		%9b
	   JMP   TrapUnhandled		%9c
	   JMP   TrapUnhandled		%9d
	   JMP   TrapUnhandled		%9e
	   JMP   TrapUnhandled		%9f

	   JMP   TrapUnhandled		%a0
	   JMP   TrapUnhandled		%a1
	   JMP   TrapUnhandled		%a2
	   JMP   TrapUnhandled		%a3
	   JMP   TrapUnhandled		%a4
	   JMP   TrapUnhandled		%a5
	   JMP   TrapUnhandled		%a6
	   JMP   TrapUnhandled		%a7
	   JMP   TrapUnhandled		%a8
	   JMP   TrapUnhandled		%a9
	   JMP   TrapUnhandled		%aa
	   JMP   TrapUnhandled		%ab
	   JMP   TrapUnhandled		%ac
	   JMP   TrapUnhandled		%ad
	   JMP   TrapUnhandled		%ae
	   JMP   TrapUnhandled		%af
	   JMP   TrapUnhandled		%b0
	   JMP   TrapUnhandled		%b1
	   JMP   TrapUnhandled		%b2
	   JMP   TrapUnhandled		%b3
	   JMP   TrapUnhandled		%b4
	   JMP   TrapUnhandled		%b5
	   JMP   TrapUnhandled		%b6
	   JMP   TrapUnhandled		%b7
	   JMP   TrapUnhandled		%b8
	   JMP   TrapUnhandled		%b9
	   JMP   TrapUnhandled		%ba
	   JMP   TrapUnhandled		%bb
	   JMP   TrapUnhandled		%bc
	   JMP   TrapUnhandled		%bd
	   JMP   TrapUnhandled		%be
	   JMP   TrapUnhandled		%bf

	   JMP   TrapUnhandled		%c0
	   JMP   TrapUnhandled		%c1
	   JMP   TrapUnhandled		%c2
	   JMP   TrapUnhandled		%c3
	   JMP   TrapUnhandled		%c4
	   JMP   TrapUnhandled		%c5
	   JMP   TrapUnhandled		%c6
	   JMP   TrapUnhandled		%c7
	   JMP   TrapUnhandled		%c8
	   JMP   TrapUnhandled		%c9
	   JMP   TrapUnhandled		%ca
	   JMP   TrapUnhandled		%cb
	   JMP   TrapUnhandled		%cc
	   JMP   TrapUnhandled		%cd
	   JMP   TrapUnhandled		%ce
	   JMP   TrapUnhandled		%cf
	   JMP   TrapUnhandled		%d0
	   JMP   TrapUnhandled		%d1
	   JMP   TrapUnhandled		%d2
	   JMP   TrapUnhandled		%d3
	   JMP   TrapUnhandled		%d4
	   JMP   TrapUnhandled		%d5
	   JMP   TrapUnhandled		%d6
	   JMP   TrapUnhandled		%d7
	   JMP   TrapUnhandled		%d8
	   JMP   TrapUnhandled		%d9
	   JMP   TrapUnhandled		%da
	   JMP   TrapUnhandled		%db
	   JMP   TrapUnhandled		%dc
	   JMP   TrapUnhandled		%dd
	   JMP   TrapUnhandled		%de
	   JMP   TrapUnhandled		%df

	   JMP   TrapUnhandled		%e0
	   JMP   TrapUnhandled		%e1
	   JMP   TrapUnhandled		%e2
	   JMP   TrapUnhandled		%e3
	   JMP   TrapUnhandled		%e4
	   JMP   TrapUnhandled		%e5
	   JMP   TrapUnhandled		%e6
	   JMP   TrapUnhandled		%e7
	   JMP   TrapUnhandled		%e8
	   JMP   TrapUnhandled		%e9
	   JMP   TrapUnhandled		%ea
	   JMP   TrapUnhandled		%eb
	   JMP   TrapUnhandled		%ec
	   JMP   TrapUnhandled		%ed
	   JMP   TrapUnhandled		%ee
	   JMP   TrapUnhandled		%ef
	   JMP   TrapUnhandled		%f0
	   JMP   TrapUnhandled		%f1
	   JMP   TrapUnhandled		%f2
	   JMP   TrapUnhandled		%f3
	   JMP   TrapUnhandled		%f4
	   JMP   TrapUnhandled		%f5
	   JMP   TrapUnhandled		%f6
	   JMP   TrapUnhandled		%f7
	   JMP   TrapUnhandled		%f8
	   JMP   TrapUnhandled		%f9
	   JMP   TrapUnhandled		%fa
	   JMP   TrapUnhandled		%fb
	   JMP   TrapUnhandled		%fc
	   JMP   TrapUnhandled		%fd
	   JMP   TrapUnhandled		%fe
	   JMP   TrapUnhandled		%ff



%         The individual Trap routines

%	 	required Traps

TrapHalt	GETA	$0,3F
		SWYM	$0,5		% tell the debugger
		JMP	2F

1H		SYNC	4		% go to power save mode	
2H		GET	$0,rQ
		BZ	$0,1B
		PUSHJ	$0,DHandler
		JMP	2B              % and loop
3H		BYTE	"DEBUG Program terminated",0		


TrapUnhandled	GETA	$0,1F
		SWYM	$0,5		% tell the debugger
		POP	0,0
1H		BYTE	"DEBUG Unhandled TRAP",0		

TrapNotImplemented	GETA	$0,1F
		SWYM	$0,5		% tell the debugger
		NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0
1H		BYTE	"DEBUG TRAP not implemented",0	

%		MMIXware Traps

TrapFputs 	AND     $0,$0,#0FF    %get the Z value 
        	BZ      $0,1F     %this is stdin
        	CMP     $1,$0,2
        	BNP     $1,4F     %this is stdout or stderr
%       	this is a file 
		JMP	TrapNotImplemented

%       	Fputs to the screen
4H      	GET	$0,rBB    %get the $255 parameter
		GET	$1,rJ
		JMP 	2F
3H		PUSHJ	$2,ScreenC
        	ADD	$0,$0,1
2H		LDBU	$3,$0,0
        	BNZ     $3,3B
		PUT	rJ,$1
1H		POP	0,0
	
TrapFopen IS	TrapNotImplemented

TrapFclose IS	TrapNotImplemented

TrapFread IS	TrapNotImplemented

TrapFgets AND     $0,$0,#0FF    %get the Z value 
        BNZ      $0,TrapNotImplemented
%       this is stdin
%	read from stdin = keyboard
1H      GET	$1,rBB    %get the $255 parameter: buffer, size
	LDO	$2,$1,0   %buffer
        LDO     $3,$1,8   %size
	SET	$0,0	  %number of chars read
	GET	$4,rJ	  %prepare for subroutine
	JMP	1F
%	loop
2H	PUSHJ	$5,KeyboardC	% read blocking from the keyboard
	STBU	$5,$2,0
	ADDU	$2,$2,1
	SUB	$3,$3,1
	ADD	$0,$0,1
	CMP	$6,$5,10	%newline
	BZ	$6,2F
1H	BP	$3,2B
% 	size is zero, done
2H	PUT	rBB,$0     %the result is returned with resume 1
	PUT	rJ,$4
	POP	0,0


TrapFgetws IS	TrapNotImplemented


TrapFwrite AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	JMP	TrapNotImplemented


%       Fwrite to the screen
4H      GET	$0,rBB    %get the $255 parameter
	LDO	$1,$0,8   %size
	LDO	$0,$0,0   %buffer
	GET	$2,rJ
	JMP 	2F

3H	LDBU    $4,$0,0
	ADD	$0,$0,1
	PUSHJ	$3,ScreenC
	SUB	$1,$1,1
2H      BP      $1,3B
	PUT	rJ,$2
1H	POP	0,0

TrapFputws IS	TrapNotImplemented

TrapFseek  IS	TrapNotImplemented

TrapFtell  IS	TrapNotImplemented


%	optional Traps

%		Set the bits of the sevensegment display
TrapSSet	SETH	$2,IOBase	base address
		GET	$0,rBB		bits for the display
		STOU	$0,$2,sevenseg
		POP	0,0


%		Wait for a specified ammount of time
TrapTSleep	SET	$1,1		Timer Event
		SL	$1,$1,21
		SETH	$2,IOBase	base address
		GET	$0,rBB		time in ms
		SL	$0,$0,32	intervall is 0
		STOU	$0,$2,timer+#10
		JMP	1F

2H		SYNC	4		%wait idle for an interrupt
1H		GET	$0,rQ		
		AND	$3,$0,$1
		BZ	$3,2B	
		SET	$3,0
		STTU	$3,$2,timer+#10 %reset timer
		GET	$0,rQ		  % get rQ again, to read additional timer interrupts
		ANDN	$0,$0,$1	  %clear timer Interrupt		
		PUT	rQ,$0
		POP	0,0
		
%		Get the current date in format YYYYMMDW
TrapTDate	SETH    $1,IOBase	
		LDOU	$0,$1,timer	  %YYMDXXXW
		AND	$2,$0,#FF	  %W
		SRU	$6,$0,32
		AND	$3,$6,#FF	  %D
		SRU	$6,$6,8
		AND	$4,$6,#FF	  %M
		SRU	$5,$6,8	  %YY
		
		SL	$3,$3,8	 
		OR	$2,$2,$3
		SL	$4,$4,16
		OR	$2,$2,$4
		SL	$5,$5,32
		OR	$2,$2,$5
		PUT	rBB,$2		  %YYYYMMDW
		POP	0,0



%		Read the current Time in ms 
TrapTTimeOfDay	SETH    $1,IOBase	
		LDTU	$0,$1,timer+#0C
		PUT	rBB,$0
		POP	0,0

%		Put one pixel on the graphics display. 
%		In $255 we havein the Hi 32 bit the RGB value
%               and in the low 32 bit the x y value as two WYDEs
%		we assume a 640x480 screen
TrapGPutPixel 	GET	$0,rBB		%get the $255 parameter: RGB, x, y
	      	SRU     $1,$0,32	%RGB
              	SETH    $2,VRamBase	%base address of vram
              	SET	$3,#FFFF
              	
              	AND	$4,$0,$3	y
             	% multiply y with  640*4 = *2^6*2*(1+4) = 2^7*(1+4)
             	4ADDU	$4,$4,$4
             	SL	$4,$4,7
             	
              	SR	$5,$0,16	x
              	AND	$5,$5,$3	x
              	4ADDU	$4,$5,$4	y = 4*x+y
              	STTU	$1,$2,$4        RGB-> base+y
               	PUT	rBB,0		%the result is returned with resume 1
	      	POP	0,0
	      	
%		Wait for a mouse event and return the descriptor
TrapMWaitEvent	SET	$1,1		Mouse Event
		SL	$1,$1,19
		SETH	$2,IOBase	base address
		JMP	1F

2H		SYNC	4		%wait idle for an interrupt
1H		GET	$0,rQ		
		AND	$3,$1,$0
		BZ	$3,2B	
		ANDN	$0,$0,$1	%clear mouse Interrupt		
		PUT	rQ,$0
		
		LDO	$0,$2,mouse	%mouse status
		PUT	rBB,$0		$return via rBB in $255
		POP	0,0



%		Put one character on the graphics display. 
%		In $255 we havein the Hi 32 bit the ASCII value
%               and in the low 32 bit the x y value as two WYDEs

TrapGPutChar 	GET	$0,rBB		%get the $255 parameter: c, x, y
              	SETH    $1,IOBase	%base address of gpu -20
              	ORH	$0,#0100	% add the Write Character Command
              	STTU	$0,$1,GPU+#0c	store position
              	STHT	$0,$1,GPU
	     	POP	0,0



TrapGPutStr	GET	$0,rBB		%get the $255 point to the string
              	SETH    $1,IOBase	%base address of gpu -20
              	JMP 1F

2H		ORML	$2,#0100        % add the Write Character Command
		STT	$2,$1,GPU      	
		ADD	$0,$0,1
1H		LDBU	$2,$0,0
		BNZ	$2,2B
		POP	0,0

TrapGLine	GET	$0,rBB		%get the $255 parameter: w, x, y, 
		ORH	$0,#0300	%the draw line command
              SETH    $1,IOBase	%base address of gpu
		STO	$0,$1,GPU
		POP	0,0	

TrapGRectangle GET	$0,rBB		%get the $255 parameter: x, y, 
		ORH	$0,#0200	%the draw rectangle command
              SETH    $1,IOBase	%base address of gpu
		STO	$0,$1,GPU
		POP	0,0	
		
		
%	transfer a bit block within vram
%	at $255	we have  WYDE destwith,destheigth,destx,desty,srcx,srcy

TrapGBitBlt		GET	$0,rBB		%get the $255 parameter
              	SETH   $1,IOBase	%base address of gpu -20
              	LDO	$2,$0,0	%destwith,destheigth,destx,desty
              	STO	$2,$1,GPU+#08
              	LDTU	$2,$0,8	%destx,desty
              	ORH	$2,#04CC	Command=5, RasterOP=CC0020 (SRCCOPY)
              	ORMH	$2,#0020
              	STOU	$2,$1,GPU
              	POP	0,0

%		Set the position for the next GPutStr, GLine Operation
TrapGSetPos	GET	$0,rBB		%get the $255 parameter: x,y
              	SETH    $1,IOBase	%base address of gpu -20
              	STT	$0,$1,GPU+#0C
	     	POP	0,0

TrapGSetTextColor	GET	$0,rBB	% background RGB, foreground RGB
              	SETH    $1,IOBase	%base address of gpu -20
              	STO	$0,$1,GPU+#18
	     	POP	0,0
	     	
TrapGSetFillColor	GET	$0,rBB	% RGB
              	SETH    $1,IOBase	%base address of gpu -20
              	STT	$0,$1,GPU+#20
	     	POP	0,0


TrapGSetLineColor	GET	$0,rBB	% RGB
              	SETH    $1,IOBase	%base address of gpu -20
              	STT	$0,$1,GPU+#24
	     	POP	0,0


%	transfer a bit block from normal memory into vram
%	at $255	we have:  WYDE with,heigth,destx,desty; OCTA srcaddress
TrapGBitBltIn		GET	$0,rBB		%get the $255 parameter
              	SETH   $1,IOBase	%base address of gpu -20
              	LDO	$2,$0,0	%with,heigth,destx,desty
              	STO	$2,$1,GPU+#08
              	GET	$3,rJ
              	LDO	$5,$0,8	
              	PUSHJ	$4,virt2phys
              	PUT    rJ,$3
              	STO	$4,$1,GPU+#10
              	SETH	$3,#05CC	Command=5, RasterOP=CC0020 (SRCCOPY)
              	ORMH	$3,#0020
              	SET	$4,1
              	SL	$4,$4,20
              	GET	$2,rQ
              	ANDN	$2,$2,$4
              	PUT	rQ,$2
              	STHT	$3,$1,GPU
1H              	SYNC	4
              	GET	$2,rQ
              	AND	$1,$2,$4
              	BZ	$1,1B
              	ANDN	$2,$2,$4
              	PUT	rQ,$1
              	POP	0,0

%	transfer a bit block from vram into normal memory
%	at $255	we have:  WYDE with,heigth,srcx,srcy; OCTA destaddress
TrapGBitBltOut 	GET	$0,rBB		%get the $255 parameter
              	SETH   $1,IOBase	%base address of gpu -20
              	LDO	$2,$0,0	%with,heigth,srcx,srcy
              	STO	$2,$1,GPU+#08
              	GET	$3,rJ
              	LDO	$5,$0,8	%srcaddress	
              	PUSHJ	$4,virt2phys
              	PUT    rJ,$3
              	STO	$4,$1,GPU+#10               	
              	SRU	$3,$2,61	Segment No
              	ANDNH  $2,#E000      remove segment
			SET	$4,#001a      Data Segment (default)
              	CSZ	$4,$3,#02	Text Segment
              	SL	$4,$4,12
              	ORMH	$4,#0001	Ram Base
              	ADD	$4,$4,$2	Translate to physical using data VTC
          		STO	$4,$1,GPU+#10
              	SETH	$3,#06CC	Command=6, RasterOP=CC0020 (SRCCOPY)
              	ORMH	$3,#0020
              	SET	$4,1
              	SL	$4,$4,20
              	GET	$2,rQ
              	ANDN	$2,$2,$4
              	PUT	rQ,$2
              	STHT	$3,$1,GPU
1H              	SYNC	4
              	GET	$2,rQ
              	AND	$1,$2,$4
              	BZ	$1,1B
              	ANDN	$2,$2,$4
              	PUT	rQ,$1
              	POP	0,0


%       The ROM Page Table
%       the table maps each segement with up to 1024 pages
%	currently, the first page is system rom, the next four pages are for
%       text, data, pool, and stack. then there is mor bios code.
%       The page tables imply the following RAM Layout

%	The RAM Layout

%       the ram layout uses the small memmory model (see memory.howto)
%       8000000100000000    first page for OS, layout see below
%       Next the  pages for the user programm


%       free space starts at 8000000100032000

	LOC	#8000000000002000	%The start is fixed in mmix-sim.ch
%       Text Segment 12 pages = 96kByte
PageTab OCTA	#0000000100002007	%text, should be ...001 execute only
   	OCTA	#0000000100004007 
   	OCTA	#0000000100006007 
   	OCTA	#0000000100008007 
   	OCTA	#000000010000a007 
   	OCTA	#000000010000c007 
   	OCTA	#000000010000e007 
   	OCTA	#0000000100010007
   	OCTA	#0000000100012007
   	OCTA	#0000000100014007
	OCTA	#0000000100016007 
	OCTA	#0000000100018007  
   	 
%       Data Segment 8 pages = 64 kByte
	LOC     (@&~#1FFF)+#2000	%data
	OCTA	#000000010001a006  
	OCTA	#000000010001c006  
	OCTA	#000000010001e006  
	OCTA	#0000000100020006  
	OCTA	#0000000100022006  
	OCTA	#0000000100024006  
	OCTA	#0000000100026006  
	OCTA	#0000000100028006  

%	Pool Segment 2 pages = 16 kByte
	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010002a006	%pool
	OCTA	#000000010002c006  
	
%	Stack Segment 2+2 pages = 32 kByte
	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010002e006	%stack
	OCTA	#0000000100030006  
	OCTA	#0000000100032006  
	OCTA	#0000000100034006  
	OCTA	#0000000100036006  
	OCTA	#0000000100038006  
	OCTA	#000000010003a006  
	OCTA	#000000010003c006  
	OCTA	#000000010003e006  
	OCTA	#0000000100040006  

	LOC	(@&~#1FFF)+#2000-2*8	
	OCTA	#0000000100042006	%gcc memory stack < #6000 0000 0080 0000
	OCTA	#0000000100044006  

	LOC	(@&~#1FFF)+#2000
	

%       initialize the memory management
memory	SETH    $0,#1234	%set rV register
	ORMH    $0,#0D00      
	ORML    $0,#0000
	ORL     $0,#2000
	PUT	rV,$0

	SETH    $0,#8000       %$0 physical address of RAM
        ORMH    $0,#0001 
        
        SET	$1,$0
	ORML	$1,#0004
        ORL     $1,#6000       %address of first empty page
	STO	$1,$0,0        %initialize FreeSpace
	POP     0,0

%       TRAP handler for page faults (not yet implemented)
FTrapPageFault		GETA	$0,1F
			SWYM	0,5		% tell the debugger
			POP	0,0
1H			BYTE    "DEBUG Page fault - not yet implemented",0

% Translate virtual adresses to physical - very simple version

virt2phys	SRU	$1,$0,61
		AND	$1,$1,3    segment number
		SLU	$1,$1,10   page table offset
		GETA	$2,PageTab
		ADD	$1,$1,$2
		SRU	$2,$0,10   8 * page number
		SET	$3,#1FFF   13 bit
		AND	$2,$2,$3
		LDO  	$2,$1,$2   PTE
              ANDN	$2,$2,7    remove permission bits
		AND    $1,$0,$3
		ADDU   $0,$0,$2
		POP	1,0
		
	       
		
		


%	First Page in RAM: reserved for the OS.
%	The layout follows below.
%	  .section	.bss,"aw",@nobits
%	  .global	FreeSpace
% RAMSTART    LOC	#8000000100000000

	