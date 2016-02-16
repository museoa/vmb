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
	

%         The individual Trap routines

		.global TrapUnhandled

TrapUnhandled	SWYM	5		        % tell the debugger
		POP	0,0

		.global TrapHalt
	
TrapHalt	NEG	$0,1            %  enable interrupts
  		PUT	rK,$0
1H		SYNC	4		%go to power save mode
		JMP	1B              % and loop idle
		POP	0,0		% we never get here


		.global	TrapFputs
	
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

		.global	TrapFopen
	
TrapFopen 	AND     $6,$0,#0FF	% get the Z value is the handle
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0               % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$5,rBB    %get the $255 parameter
		LDO	$7,$5,0   %the name string
		LDO	$8,$5,8	  %the mode number
		PUSHJ   $5,fat32_fopen
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

		.global	TrapFclose


TrapFclose	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0

		.global	TrapFread


TrapFread	AND     $6,$0,#0FF    %get the Z value
		CMP	$1,$6,3
		BN	$1,TrapFgets	for stdin, stdout, stderr do like fgets

		% this is a file handle
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$5,rBB    %get the $255 parameter
		LDO	$7,$5,0   %the buffer
		LDO	$8,$5,8	  %the size
		PUSHJ   $5,fat32_fread
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

		.global	TrapFgets
	
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

		.global	TrapFgetws

TrapFgetws 	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0


		.global	TrapFwrite

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


		.global	TrapFputws
	
		
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

		.global	TrapFseek

TrapFseek 	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0

		.global	TrapFtell


TrapFtell	NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0



		.global	TrapGPutPixel
	
%		Put one pixel on the graphics display. 
%		In $255 we havein the Hi 32 bit the offset
%               and in the low 24 bit the RGB value 
TrapGPutPixel GET	$0,rBB		%get the $255 parameter: address and RGB
	      SRU       $1,$0,32	%offset
              SETH      $2,#8002	%base address of vram
              STTU      $0,$2,$1
              PUT	rBB,0		%the result is returned with resume 1
	      POP	0,0

