	.section    .text,"ax",@progbits
	.global FHandler

FHandler	GET	$0,rXX
		BNN	$0,Ropcode
		SRU	$1,$0,24       
		AND	$1,$1,#FF		%the opcode
		BZ	$1,TrapInstruction		

		POP	0,0			%not a TRAP and ropcode<0
       
Ropcode		SRU	$0,$0,56		%the ropcode
		BZ	$0,PageFault		%0 means page fault
		CMP	$1,$0,2         
		BZ      $1,Emulate		%2 means emulate the instruction
		CMP	$1,$0,3	        
		BZ	$1,Virtual		%page table translation in software

TrapUnhandled   GETA    $0,1F
                SWYM    0,5            % tell the debugger
                POP     0,0
1H              BYTE    "DEBUG Unhandled TRAP",0

%       TRAP handler for page faults (not yet implemented)       	
PageFault	SWYM	5		% tell the debugger
		POP     0,0              

%       Emulate the instruction
Emulate		SWYM	5		% tell the debugger
		POP     0,0              

%	Do pagetable translation in software
Virtual		SET	$0,#1234		%the dummy physical address
		PUT	rZZ,$0			%thats where the translation is suposed to go
		POP     0,0

%       Handle a Trap Instruction
TrapInstruction SRU	$1,$0,8
		AND	$1,$1,#FF		%the Y value (the function code)
	        CMP     $3,$1,#1F
		BP	$3,TrapUnhandled	% in the moment we handle only very few Traps
		AND	$1,$1,#1F    
		SL	$1,$1,2
		GETA	$2,FTrapTable
		GO	$2,$2,$1		%Jump into the Trap Table
	
FTrapTable	JMP   TrapHalt       %0
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
		JMP   TrapUnhandled  %c
		JMP   TrapUnhandled  %d
		JMP   TrapUnhandled  %e
		JMP   TrapUnhandled  %f
		JMP   TrapGPutPixel %10
		JMP   TrapUnhandled %11
		JMP   TrapUnhandled %12
		JMP   TrapUnhandled %13
		JMP   TrapUnhandled %14
		JMP   TrapUnhandled %15
		JMP   TrapUnhandled %16
		JMP   TrapUnhandled %17
		JMP   TrapUnhandled %18
		JMP   TrapUnhandled %19
		JMP   TrapUnhandled %1a
		JMP   TrapUnhandled %1b
		JMP   TrapUnhandled %1c
		JMP   TrapUnhandled %1d
		JMP   TrapUnhandled %1e
		JMP   TrapUnhandled %1f

%         The individual Trap routines
	
TrapHalt        GETA    $0,2F
                SWYM    0,5            % tell the debugger
                NEG     $0,1            % enable interrupts
                PUT     rK,$0
1H              SYNC    4               % go to power save mode
                JMP     1B              % and loop idle
2H              BYTE    "DEBUG Program terminated",0

TrapFputs 	AND     $0,$0,#0FF    %get the Z value 
        	BZ      $0,1F     %this is stdin
        	CMP     $1,$0,2
        	BNP     $1,4F     %this is stdout or stderr

%       	this is a file 
		NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0

%       	Fputs to the screen
4H      	GET	$0,rBB    %get the $255 parameter
		GET	$1,rJ
		JMP 	2F
3H		PUSHJ	$2,ScreenC
        	ADD	$0,$0,1
2H		LDB	$3,$0,0
        	BNZ     $3,3B
		PUT	rJ,$1
1H		POP	0,0

TrapFopen	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0


TrapFclose	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0



TrapFread	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0


TrapFgets AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
%       this is stdout stderr or a file
	NEG	$0,1
	PUT	rBB,$1     %the error code is returned with resume 1
	POP	0,0

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


TrapFgetws 	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0



TrapFwrite AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	NEG	$0,1
	PUT	rBB,$1     %the error code is returned with resume 1
	POP	0,0


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

	
TrapFputws  AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	NEG	$0,1
	PUT	rBB,$1     %the error code is returned with resume 1
	POP	0,0

%       Fputsw to the screen
4H      GET	$0,rBB    %get the $255 parameter
	GET	$1,rJ
	JMP 	2F
3H	PUSHJ	$2,ScreenC
        ADD	$0,$0,2
2H	LDWU	$3,$0,0
        BNZ     $3,3B
	PUT	rJ,$1
1H	POP	0,0


TrapFseek 	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0


TrapFtell	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0


%		Put one pixel on the graphics display. 
%		In $255 we havein the Hi 32 bit the offset
%               and in the low 24 bit the RGB value 
TrapGPutPixel GET	$0,rBB		%get the $255 parameter: address and RGB
	      SRU       $1,$0,32	%offset
              SETH      $2,#8002	%base address of vram
              STTU      $0,$2,$1
              PUT	rBB,0		%the result is returned with resume 1
	      POP	0,0

