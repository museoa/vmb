	.section    .text,"ax",@progbits
	.global FHandler

FHandler	GET	$0,rXX
		BNN	$0,TrapUnhandled
		SRU	$1,$0,24       
		AND	$1,$1,#FF         
		BNZ	$1,TrapUnhandled  		
		SRU	$1,$0,8
		AND	$1,$1,#FF	%the Y value (the function code)
		SL	$1,$1,2
		GETA	$2,FTrapTable
		GO	$2,$2,$1		%Jump into the Trap Table
	


%         The individual Trap routines

		.global TrapUnhandled

TrapUnhandled   GETA    $0,1F
                SWYM    0,5            % tell the debugger
                POP     0,0
1H              BYTE    "DEBUG Unhandled TRAP",0

		.global TrapHalt

TrapHalt        GETA    $0,2F
                SWYM    0,5            % tell the debugger
		SET	$0,#e0
		PUT	rG,$0               % allocate 32 global registers for gcc
		GETA	$254,OSStackStart
		SET	$253,0              % the frame pointer for gcc
		PUSHJ	$0,fat32_shutdown
        	JMP	2F	
1H              SYNC    4               % go to power save mode
2H		GET	$0,rQ
		BZ	$0,1B
		PUSHJ	$0,DHandler
                JMP     2B              % and loop idle
	
2H              BYTE    "DEBUG Program terminated",0
	
		.global	TrapFputs
	
TrapFputs 	AND     $8,$0,#0FF    %get the Z value 
        	BZ      $8,1F     %this is stdin
        	CMP     $1,$8,2
        	BNP     $1,4F     %this is stdout or stderr

%       	this is a file 
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$7,rBB    %get the $255 parameter string
		PUSHJ   $6,fat32_fputs
		PUT	rBB,$6     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
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
	
TrapFopen 	AND     $8,$0,#0FF	% get the Z value is the handle
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0               % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$5,rBB    %get the $255 parameter
		LDO	$6,$5,0   %the name string
		LDO	$7,$5,8	  %the mode number
		PUSHJ   $5,fat32_fopen
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

		.global	TrapFclose


TrapFclose	AND     $6,$0,#0FF	% get the Z value is the handle
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0           % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
		PUSHJ   $5,fat32_fclose
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0


	        .global	TrapFread



TrapFread	AND     $10,$0,#0FF    %get the Z value
		CMP	$1,$10,3
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
		LDO	$8,$5,0   %the buffer
		LDO	$6,$5,8  %the size
	        SET	$9,$6
		PUSHJ   $7,fat32_fread
		SUB	$7,$7,$6   % return n-size like MMIXware does
		PUT	rBB,$7     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

		.global	TrapFgets
	
TrapFgets 	AND     $8,$0,#0FF    %get the Z value 
        	BZ      $8,1F     %this is stdin
        	CMP     $1,$8,3     
        	BN      $1,2F         for stdout, stderr error

%		this is a file	
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$5,rBB    %get the $255 parameter
		LDO	$6,$5,0   %the buffer
		LDO	$7,$5,8  %the size
		PUSHJ   $5,fat32_fgets
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0
	
%       this is stdout stderr 
2H	NEG	$0,1
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


TrapFgetws 	AND     $8,$0,#0FF    %get the Z value 
        	BZ      $8,1F     %this is stdin
        	CMP     $1,$8,3     
        	BN      $1,2F         for stdout, stderr error

%		this is a file	
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$5,rBB    %get the $255 parameter
		LDO	$6,$5,0   %the buffer
		LDO	$7,$5,8  %the size
		PUSHJ   $5,fat32_fgetws
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0
	
%       this is stdout stderr 
2H	NEG	$0,1
	PUT	rBB,$1     %the error code is returned with resume 1
	POP	0,0

%	read from stdin = keyboard
1H      GET	$1,rBB    %get the $255 parameter: buffer, size
	LDO	$2,$1,0   %buffer
        LDO     $3,$1,8   %size
	SET	$0,0	  %number of chars read
	GET	$4,rJ	  %prepare for subroutine
	SET	$7,0
	JMP	1F
%	loop
2H	PUSHJ	$5,KeyboardC	% read blocking from the keyboard
	STBU	$5,$2,0
	STBU	$7,$2,1
	ADDU	$2,$2,2
	SUB	$3,$3,1
	ADD	$0,$0,1
	CMP	$6,$5,10	%newline
	BZ	$6,2F
1H	BP	$3,2B
% 	size is zero, done
2H	PUT	rBB,$0     %the result is returned with resume 1
	PUT	rJ,$4
	POP	0,0


		.global	TrapFwrite


TrapFwrite 	AND     $9,$0,#0FF    %get the Z value
		BZ	$9,1F	      this is stdin
        	CMP     $1,$9,3     
        	BN      $1,4F         for stdout, stderr output to the screen


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
		LDO	$5,$5,8	  %the size
	        SET	$8,$5   
		PUSHJ   $6,fat32_fwrite
	        SUB	$6,$6,$5
		PUT	rBB,$6     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

%       	Fwrite to the screen
4H      	GET	$2,rJ
		GET	$0,rBB    %get the $255 parameter
	        LDO	$1,$0,8   the size
	        LDO     $0,$0,0   the buffer
		JMP 	2F
3H		LDB	$4,$0,0
		ADD	$0,$0,1
		PUSHJ	$3,ScreenC
2H        	SUB	$1,$1,1
        	BNN     $1,3B
		PUT	rBB,0
		PUT	rJ,$2
1H		POP	0,0

		.global	TrapFputws
	
TrapFputws 	AND     $8,$0,#0FF    %get the Z value 
        	BZ      $8,1F     %this is stdin
        	CMP     $1,$8,2
        	BNP     $1,4F     %this is stdout or stderr

%       	this is a file 
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$7,rBB    %get the $255 parameter string
		PUSHJ   $6,fat32_fputws
		PUT	rBB,$6     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

%       	Fputs to the screen
4H      	GET	$0,rBB    %get the $255 parameter
		GET	$1,rJ
		JMP 	2F
3H		PUSHJ	$2,ScreenC
        	ADD	$0,$0,2
2H		LDB	$3,$0,0
	        LDB	$4,$0,1
		OR	$4,$4,$3
        	BNZ     $4,3B
		PUT	rJ,$1
1H		POP	0,0

		.global	TrapFseek

TrapFseek 	AND     $9,$0,#0FF    %get the Z value 
        	CMP     $1,$9,2
        	BNP     $1,1F     %this is stdin, stdout or stderr

%       	this is a file 
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
	      	GET	$7,rBB    %get the $255 parameter string
		BN	$7,2F
		% offset is positive seek set
		SET	$8,0	  0 is SEEK_SET
		PUSHJ   $6,fat32_fseek
		JMP	3F
	
		% offset is negative seek end
2H		SET	$8,2	  2 is SEEK_END
		ADD	$7,$7,1   offset+1, to adjust for different calling convention
		PUSHJ   $6,fat32_fseek
	
3H		PUT	rBB,$6     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

1H		NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0

		.global	TrapFtell

TrapFtell	AND     $6,$0,#0FF    %get the Z value 
        	CMP     $1,$6,2
        	BNP     $1,1F     %this is stdin, stdout or stderr

%       	this is a file 
	        GET     $0,rJ		% the return address
		SET	$1,$255		% save global registers destroyed by gcc
		SET	$2,$254
		SET	$3,$253
		GET	$4,rG
		PUT	rG,#e0          % allocate 32 global registers for gcc
	
		GETA	$254,OSStackStart
		SET	$253,0
		PUSHJ   $5,fat32_ftell
		PUT	rBB,$5     %the error code is returned with resume 1
		SET	$255,$1   % restore user gcc stack and globals
		SET	$254,$2
		SET	$253,$3
		PUT	rG,$4
		PUT	rJ,$0
		POP	0,0

1H		NEG	$0,1
		PUT	rBB,$1     %the error code is returned with resume 1
		POP	0,0



