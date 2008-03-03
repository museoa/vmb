%	this is the MMIX BIOS
%	it is considert to be the ROM loaded 
%	at physical address 0000000000000000
%	used with 
%	virtual address 8000000000000000

	.section    .text,"ax",@progbits		
	LOC	#8000000000000000
	
% page table setup (see small model in address.howto)
Main	IS	0
Boot	GETA	$0,DTrap     set dynamic- and forced-trap  handler
	PUT	rTT,$0
	GETA	$0,FTrap
	PUT	rT,$0
	PUSHJ   $255,selftest
        PUSHJ   $255,memory
        SET	$0,0
	PUT     rQ,$0		 %    clear interrupts

%	here we start a loaded program
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
	NEG	$255,1	  % enable interrupt $255->rK with resume 1
	RESUME	1
	
DHandler	GET 	$0,rQ
	SUBU	$1,$0,1      %from xxx...xxx1000 to xxx...xxx0111
	SADD	$2,$1,$0     %position of lowest bit
	ANDN	$1,$0,$1     %the lowest bit
        ANDN	$1,$0,$1     %delete lowest bit
	PUT	rQ,$1        %and return to rQ
	SLU	$2,$2,2      %scale
        GETA	$1,DTrapTable %and jump
	GO	$1,$1,$2


DTrapTable JMP DTrapUnhandled  %0
	   JMP DTrapUnhandled  %1
           JMP DTrapUnhandled  %2
           JMP DTrapUnhandled  %3
           JMP DTrapPageFault  %4
           JMP DTrapUnhandled  %5
           JMP DTrapUnhandled  %6
           JMP DTrapUnhandled  %7
           JMP DTrapUnhandled  %8
           JMP DTrapUnhandled  %9
           JMP DTrapUnhandled  %10
           JMP DTrapUnhandled  %11
           JMP DTrapUnhandled  %12
           JMP DTrapUnhandled  %13
           JMP DTrapUnhandled  %14
           JMP DTrapUnhandled  %15
           JMP DTrapUnhandled  %16
           JMP DTrapKey        517
           JMP DTrapScreen     %18
           JMP DTrapUnhandled  %19
           JMP DTrapUnhandled  %20
           JMP DTrapUnhandled  %21
           JMP DTrapUnhandled  %22
           JMP DTrapUnhandled  %23
           JMP DTrapUnhandled  %24
           JMP DTrapUnhandled  %25
           JMP DTrapUnhandled  %26
           JMP DTrapUnhandled  %27
           JMP DTrapUnhandled  %28
           JMP DTrapUnhandled  %29
           JMP DTrapUnhandled  %30
           JMP DTrapUnhandled  %31
           JMP DTrapUnhandled  %32
           JMP DTrapUnhandled  %33
           JMP DTrapUnhandled  %34
           JMP DTrapUnhandled  %35
           JMP DTrapUnhandled  %36
           JMP DTrapUnhandled  %37
           JMP DTrapUnhandled  %38
           JMP DTrapUnhandled  %39
           JMP DTrapUnhandled  %40
           JMP DTrapUnhandled  %41
           JMP DTrapUnhandled  %42
           JMP DTrapUnhandled  %43
           JMP DTrapUnhandled  %44
           JMP DTrapUnhandled  %45
           JMP DTrapUnhandled  %46
           JMP DTrapUnhandled  %47
           JMP DTrapUnhandled  %48
           JMP DTrapUnhandled  %49
           JMP DTrapUnhandled  %50
           JMP DTrapUnhandled  %51
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
           JMP DTrapUnhandled  %64  rQ was zero


console   IS	#8001             %   hi wyde of console	

DTrapKey	SETH	$1,console    
	LDO	$2,$1,0		%keyboard status/data
	BN	$2,1F	
	SR	$3,$2,32
	AND	$3,$3,#FF
	BZ	$3,1F	
	AND	$2,$2,#FF
	STO	$2,$1,8
	CMP	$2,$2,#0D		%carriage return
	BNZ	$2,1F		
	SET	$2,#0A		%line feed
	STO	$2,$1,8
1H	POP	0,0

DTrapScreen	SETH    $0,#8000
        ORMH	$0,#0001	%address of bios ram
	LDT	$1,$0,8	        %index screen buffer start
	LDT	$2,$0,12        % index screen buffer limit
	CMP	$3,$1,$2
	BZ	$3,1F		%buffer empty
	ADDU	$3,$0,16	 %       screen buffer address
	LDBU	$3,$3,$1
	SETH	$4,console 
	STO	$3,$4,8                 
	ADD     $1,$1,1
	AND	$1,$1,#FF       %256 byte wrap around buffer
	STTU	$1,$0,8
1H	POP	0,0

DTrapUnhandled	SWYM	5               % inform the debugger
		POP	0,0
 

FTrap	PUSHJ	$255,FHandler
	PUT	rJ,$255
	NEG	$255,1	  %enable interrupt $255->rK with resume 1
	RESUME	1

FHandler	GET     $0,rXX
		BNN	$0,Ropcode
        SRU     $1,$0,24       
	AND	$1,$1,#FF	%the opcode
	BZ	$1,Trap		

	POP     0,0             %not a TRAP and ropcode<0
       
Ropcode	SRU	$0,$0,56	%the ropcode
	BZ	$0,DTrapPageFault    %0 means page fault
	SUB	$1,$0,2         
	BZ      $1,Emulate      %2 means emulate the instruction
	CMP	$1,$0,3	        
	BZ	$1,Virtual      %page table translation in software

	POP     0,0             %ignore the rest


	

Emulate POP     0,0		%not implemented

Virtual SET	$0,#1230         %the dummy physical address
	PUT	rZZ,$0
	POP     0,0


Trap    GETA	$2,FTrapTable
	SRU	$1,$0,8
	AND	$1,$1,#FF       %the Y value (the function code)
        CMP     $3,$1,#1F
	BP	$3,TrapUnhandled % in the moment we handle only very few Traps
	AND	$1,$1,#1F    
	SL	$1,$1,2
1H	GO	$2,$2,$1  %Jump into the Trap Table
	
FTrapTable JMP   TrapHalt      %0
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
	  JMP   TrapGPutPixel %10 call as TRAP 0,#10,0, $255 has offset,0RGB
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

%         The Trap routines


TrapHalt NEG	$0,1            %  enable interrupts
  	PUT	rK,$0
1H	SYNC	4		%go to power save mode
	JMP	1B              % and loop idle

TrapFputs AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
        GET	$0,rJ
	SETML	$3,#0001 %offset for buffer
	ADD	$3,$3,$1  %address of disk buffer
        GET	$4,rBB    %get the $255 parameter: address of string
	PUSHJ	$2,strcpy	
	STTU	$2,$1,20   %nmeb
        SET     $2,#1
	STTU	$2,$1,16   %size
	SET	$2,4
	STO	$2,$1,8    %Fwrite to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
	PUT	rJ,$0
	POP	0,0

%       Fputs to the screen
4H      GET	$0,rBB    %get the $255 parameter
	GET	$1,rJ
	JMP 	2F
3H	PUSHJ	$2,ScreenC
        ADD	$0,$0,1
2H	LDB	$3,$0,0
        BNZ     $3,3B
	PUT	rJ,$1
1H	POP	0,0


%       memcpy utility
1H	LDBU	$3,$1,0  %copy $2 bytes from $1 to $0
	STBU    $3,$0,0
        ADD	$0,$0,1
	ADD	$1,$1,1
	SUB	$2,$2,1
memcpy	BP	$2,1B
	POP	0,0

%       octacpy utility  %like memcpy but with a multiple of octas
1H	LDOU	$3,$1,0  %copy $2 bytes from $1 to $0
	STOU    $3,$0,0
        ADD	$0,$0,8
	ADD	$1,$1,8
	SUB	$2,$2,8
octacpy	BP	$2,1B
	POP	0,0
	
%       strcpy utility
strcpy	SET	$3,0
	JMP	2F
1H      ADD	$0,$0,1
	ADD	$1,$1,1
	ADD	$3,$3,1   %counting bytes
2H	LDBU    $2,$1,0
	STBU    $2,$0,0   %bytes from $1 to $0 until zero byte returns size
	BNZ	$2,1B
	SET	$0,$3
	POP	1,0

%       strcpy utility for wide characters
strcpyw	SET	$3,0
	JMP	2F
1H      ADD	$0,$0,2
	ADD	$1,$1,2
	ADD	$3,$3,2   %counting bytes
2H	LDWU    $2,$1,0
	STWU    $2,$0,0   %bytes from $1 to $0 until zero byte returns size
	BNZ	$2,1B
	SET	$0,$3
	POP	1,0
	
TrapFopen AND     $0,$0,#0FF    %get the Z value 
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdin, stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
        GET	$0,rJ
	SET	$3,#20    %offset for filename
	ADD	$3,$3,$1  %address of filename
        GET	$4,rBB    %yget the $255 parameter: address of string
        LDO	$4,$4,0   %address of filename
	PUSHJ	$2,strcpy	
        GETA    $2,mmixmodes
	GET     $3,rBB
	LDO     $3,$3,8    %the mode
        SLU     $3,$3,2    %scalein the index for mmixmodes
        LDTU    $3,$2,$3
	STTU	$3,$1,#10   %mode arg1
	SET	$2,1
	STO	$2,$1,8    %Fopen to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
	PUT	rJ,$0
1H	POP	0,0

mmixmodes BYTE 'r',0,0,0   %TextRead
          BYTE 'w',0,0,0   %TextWrite
	  BYTE 'r','b',0,0  % BinaryRead
          BYTE 'w','b',0,0   %BinaryWrite
	  BYTE 'w','b','+',0  % BinaryReadWrite


TrapFclose AND     $0,$0,#0FF  %  get the Z value 
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdin, stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
	SET	$2,2
	STO	$2,$1,8    %Fclose to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
1H	POP	0,0


TrapFread AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,4F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor

        GET	$2,rBB    %get the $255 parameter: address of string
	LDO	$3,$2,0   %buffer
        LDO     $5,$2,8   %size
	STTU	$5,$1,20  % nmeb
        SET     $2,#1
	STTU	$2,$1,16   %size
	SET	$2,3
	STO	$2,$1,8    %Fread to command
	SETML	$4,#0001  %offset for buffer
	ADD	$4,$4,$1   %address of disk buffer
	LDTU    $1,$1,28   %result
        GET	$0,rJ
	PUSHJ	$2,memcpy
	PUT	rBB,$1    %the result is returned with resume 1
	PUT	rJ,$0
1H	POP	0,0

%       Fread from the keyboard
4H      POP	0,0

TrapFgets AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,4F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor

        GET	$2,rBB    %get the $255 parameter: address of string
	LDO	$3,$2,0   %buffer
        LDO     $5,$2,8   %size
	STTU	$5,$1,16  % size
	SET	$2,10
	STO	$2,$1,8    %Fgets to command
	SETML	$4,#0001  %offset for buffer
	ADD	$4,$4,$1   %address of disk buffer
	LDTU    $1,$1,28   %result
	BN	$1,2F
        GET	$0,rJ
	PUSHJ	$2,memcpy
2H	PUT	rBB,$1    %the result is returned with resume 1
	PUT	rJ,$0
1H	POP	0,0

%       Fread from the keyboard
4H      POP	0,0

TrapFgetws AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,4F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor

        GET	$2,rBB    %get the $255 parameter: address of string
	LDO	$3,$2,0   %ram buffer
	SETML	$4,#0001 %offset for disk buffer
	ADD	$4,$4,$1  %address of disk buffer
        LDO     $5,$2,8   %size in byte
        SET	$2,2
	STTU	$5,$1,20   %nmeb
        SET     $2,1
	STTU	$2,$1,16   %size
	SET	$2,3	   %Fread
	SET 	$6,0       %counter
	JMP     3F


5H	STO	$2,$1,8    %Fread to command
	LDTU    $1,$1,28   %result
	BN	$1,6F
	BZ	$1,7F
        LDWU	$7,$4,0    %wyde read
	STWU	$7,$3,$6
	ADD	$6,$6,2
	SUB	$5,$5,2
	CMP	$7,$6,10   %newline
	BZ	$7,7F

3H	BP	$5,5B


6H	PUT	rBB,$1    %the result is returned with resume 1
1H	POP	0,0

7H      PUT     rBB,$6
	POP	0,0

%       Fgetws from the keyboard
4H      POP	0,0




TrapFwrite AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
        GET	$0,rJ
	SETML	$3,#0001 %offset for buffer
	ADD	$3,$3,$1  %address of disk buffer
        GET	$2,rBB    %get the $255 parameter: address of string
        LDO	$4,$2,0   %buffer
        LDO     $5,$2,8   %size
	PUSHJ	$2,memcpy
        GET	$2,rBB    %get the $255 parameter: address of string
        LDO     $2,$2,8   %size
	STTU	$2,$1,20   %nmeb
        SET     $2,#1
	STTU	$2,$1,16   %size
	SET	$2,4
	STO	$2,$1,8    %Fwrite to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
	PUT	rJ,$0
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
	PUT	rJ,$1
1H	POP	0,0

TrapFputws  AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
        GET	$0,rJ
	SETML	$3,#0001 %offset for buffer
	ADD	$3,$3,$1  %address of disk buffer
        GET	$4,rBB    %get the $255 parameter: address of string
	PUSHJ	$2,strcpyw	
	STTU	$2,$1,20   %nmeb
        SET     $2,#1
	STTU	$2,$1,16   %size
	SET	$2,4
	STO	$2,$1,8    %Fwrite to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
	PUT	rJ,$0
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


TrapFseek AND     $0,$0,#0FF   % get the Z value 
        CMP     $1,$0,2
        BNP     $1,1F     %this is stdin, stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2   %base addess of file descriptor
	GET     $3,rBB     %the offset
	ZSN	$2,$3,2    %    SEEK_END=2, SEEK_SET=0
	ZSN	$4,$3,1    %in case of SEEK_END add 1 to offset
	STTU	$2,$1,20   %whence
	ADD	$2,$3,$4
	STTU	$2,$1,16   %offset
	SET	$2,5
	STO	$2,$1,8    %Fseek to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
1H	POP	0,0



TrapFtell  AND     $0,$0,#0FF    %get the Z value 
        CMP     $1,$0,2
        BNP     $1,1F    % this is stdin, stdout or stderr
%       this is a file 
	SETH	$2,#8002
	SL	$1,$0,17
	OR	$1,$1,$2  % base addess of file descriptor
	SET	$2,6
	STO	$2,$1,8    %Ftell to command
	LDTU    $2,$1,28   %result
	PUT	rBB,$2     %the result is returned with resume 1
1H	POP	0,0



TrapGPutPixel GET	$0,rBB    %get the $255 parameter: address and RGB
	      SR        $1,$0,32  %offset
              SETH      $2,#8002  %base address of vram
              STTU      $0,$1,$2
              PUT	rBB,0     %the result is returned with resume 1
	      POP	0,0


TrapUnhandled	SWYM	5		% tell the debugger
		POP	0,0


ScreenC	SETH    $1,#8000
        ORMH	$1,#0001	%address of bios ram
2H	LDTU	$2,$1,8	        %index screen buffer start
	LDTU	$3,$1,12        % index screen buffer limit
	SUB	$4,$3,$2
	BNZ	$4,1F		%buffer not empty, char->buffer

	SETH	$1,console	%try direct output
	LDO	$2,$1,8		%screen status/data
	SR	$2,$2,32	%get error and count data
	BNZ	$2,1F           %char->buffer
	STO	$0,$1,8		%direct output
	POP	0,0

1H	AND	$4,$4,#FF
	CMP	$5,$4,#FF	
	BN	$5,1F
	SYNC	4		%wait idle
	JMP	2B

1H	ADD	$2,$1,16		%address of screen buffer
	STB	$0,$2,$3
	ADD	$3,$3,1
	AND	$3,$3,#FF
	STTU	$3,$1,8
	POP	0,0

selftest POP	0,0
	GETA	$5,tstdata       %some code to test MMIX
	LDO     $0,$5,0
	SET	$5,$0
	SET	$1,#100
	STO	$5,$1,0
	LDB     $6,$1,7
        SETH    $2,#2000
	LDO     $3,$2,$1
        STO     $5,$2,$1
	LDO	$7,$2,$1
        SETH    $2,#4000
	LDO     $3,$2,$1
        STO     $5,$2,$1
	LDO	$3,$2,$1
        SETH    $2,#6000
	LDO     $3,$2,$1
        STO     $5,$2,$1
	LDO	$3,$2,$1
	POP	0,0



tstdata OCTA    #123456789ABCDEF0

%       The initial ROM Page Table

	LOC	(@&~#1FFF)+#2000
PageTab OCTA	#0000000100002007  %text, should be 2001 for execute only
   	OCTA	#000000010000A007 
   	OCTA	#000000010000C007 
   	OCTA	#000000010000E007 
   	OCTA	#0000000100010007 
   	OCTA	#0000000100012007 
   	OCTA	#0000000100014007 
   	OCTA	#0000000100016007 
	LOC     (@&~#1FFF)+#2000
	OCTA	#0000000100004006  %data
	LOC	(@&~#1FFF)+#2000
	OCTA	#0000000100006006  %pool
	LOC	(@&~#1FFF)+#2000
	OCTA	#0000000100008006  %stack
	LOC	(@&~#1FFF)+#2000
PageTabSize     IS 4                %    should be ((@-PageTab)>>13)
%       the table maps each segement with up to 1024 pages
%	currently, the first page is system rom, the next four pages are for
%       text, data, pool, and stack and
%       then 7 more pages are maped for the text segment
%       this is the a total of 12 pages. Then follows the
%       page table itself with 4 more blocks once it is copied into ram


% now an exercise in mmix page table setup
% we want to map the virtual address #5000 0000 0000 0000
% of the videoram 
% to its physical address #0005 0000 0000 0000
% and the same for the next pages up to 1MB
% the virtual Address is in the pool segment (#4000... to #5FFFF...)
% we need a different Value for rV it sould be #1278
% 1 page text 1 page data 5 pages pool and 1 page stack

% new layout after rom was copied to ram at 18000
%8000000100000000+
%18000: OCTA	#0000000100002007  text, should be 2001 for execute only
%   	OCTA	#000000010000A007 
%   	OCTA	#000000010000C007 
%   	OCTA	#000000010000E007 
%   	OCTA	#0000000100010007 
%   	OCTA	#0000000100012007 
%   	OCTA	#0000000100014007 
%   	OCTA	#0000000100016007 
%
%1A000	OCTA	#0000000100004006  data 
%
%1C000	OCTA	#0000000100006006  pool0 
%
%1E000	OCTA	#0000000000000000  pool1
%               one page
%20000	OCTA	#0000000000000000  pool2
%
%22000	OCTA	#0000000000000000  pool3
%
%24000	OCTA	#0000000000000000  pool4
%...
%24400  OCTA    #8000000100028000  PTP for #5000 0000 0000 0000 
%
%26000  OCTA    #0000000100008006  stack PTE
%
%28000	OCTA	#800000010002A000 
%
%2A000	OCTA	#800000010002C000 
%
%2C000	OCTA	#800000010002E000 
%
%2E000	OCTA	#0000000200000006  pool PTE
%2E008	OCTA	#0000000200002006  pool PTE
%2E010	OCTA	#0000000200004006  pool PTE
%... allocating 480*640*4 = 1200 kByte= 150 pages of 8kByte
%2e4A8   OCTA   #000000020012A006  pool PTE
%
% next we need rV to be 1278 0D 01000180000
% and the number of ram pages allocated grows to 25
%done. now we write the procedure to set this up



mapvideo SETH   $0,#8000       %$0 physical address of RAM
        ORMH    $0,#0001 
	SETML	$1,#0001        %offset for stack page Table
        ORL     $1,#E000
        LDO     $2,$0,$1       %PTE for stack
        STCO    0,$0,$1
	SETML	$1,#0002       %new offset for stack page Table
        ORL     $1,#6000        
	STO     $2,$0,$1

	SETML	$1,#0002       %pool4  at offset for 5000...
        ORL     $1,#4400         
	SETML   $2,#0002
	ORL     $2,#8000
        OR      $2,$2,$0     
        STO     $2,$0,$1	% PTE for 50

	SET	$1,$2
	SET	$3,#2000
	ADDU	$2,$2,$3
	STO	$2,$1,0		 %PTE for 50 00

	SET	$1,$2
	ADDU	$2,$2,$3
	STO	$2,$1,0		 %PTE for 50 00 00 
        
	SET	$1,$2
	ADDU	$2,$2,$3
	STO	$2,$1,0		 %PTE for 50 00 00 00 
        
	SET	$1,$2
	SETMH	$2,#0002
	ORL     $2,#0006
	SET	$4,150
1H	STO	$2,$1,0
	ADD	$2,$2,$3
	ADD	$1,$1,8
	SUB	$4,$4,1
	BP	$4,1B
	
	SYNC	5		%empty write buffer to update page table
	SYNC	6		%clear Virtual Address Translation cache

	SETH    $2,#1278
	ORMH    $2,#0D01	%rV register
        ORML    $2,#0001
	ORL     $2,#8000
	PUT	rV,$2

	SETH    $2,#8000	%$2 physical address of RAM
        ORMH    $2,#0001  
	STCO	25,$0,0		%number of pages used in the first octa of system ram

	POP     0,0








%       ram layout the small memmory model (see memory.howto)
%       8000000100000000    first page for OS
%       8000000100002000    page for application text segment
%       8000000100004000    page for application data segment
%       8000000100006000    page for application pool segment
%       8000000100008000    page for application stack segment
%       the above pages are located at the same place as with the 
%       pagetable in rom provided below during startup
%       needed if the native mmix simulator loads an mmo file
%       800000010000A000    pagetable for application text
%       800000010000C000    pagetable for application data
%       800000010000E000    pagetable for application pool
%       8000000100010000    pagetable for application stack
%
%       the pagetable replicates the ROM pagetable
%       but can be modified, assigning new pages on demand.


%       initialize the memory management
memory	GET	$0,rJ

	SETH    $2,#8000       %$0 physical address of RAM
        ORMH    $2,#0001 
	ORML	$2,#0001
        ORL     $2,#8000       %offset for page Table
	GETA    $3,PageTab
	SET	$4,PageTabSize %size of the page table PageTabSize
	SL      $4,$4,13       %size in byte
        PUSHJ   $1,octacpy     %copy 

	SYNC	5              %empty write buffer to update page table
	SYNC    6              %clear Virtual Address Translation cache
	SYNC    7              %clear i and d cache (not strictly necessary)

	SETH    $2,#1234
	ORMH    $2,#0D01       %rV register
	ORML    $2,#0001
	ORL     $2,#8000
	PUT	rV,$2

	SETH    $2,#8000       %$2 physical address of RAM
        ORMH    $2,#0001 
        SET     $1,16          %number of pages allocated 
	STO	$1,$2,0        %in the first octa of the system ram
	PUSHJ   $1,mapvideo
	PUT	rJ,$0
	POP     0,0

%       TRAP handler for page faults (not yet complete)
         .extern testproc
       
	
DTrapPageFault  GET $0,rYY
	  GET $1,rZZ            %lets assume this is
	  ADD $0,$0,$1          %the virtual address (as in LDO)
	  SRU $1,$0,61          %the segment in $1
	  ANDNH $0,#E000        %the offset in $0
	  GET $2,rV
          AND $5,$2,#7         %the three f bits in $5
          SLU $6,$2,64-13
          SRU $6,$6,64-10       %the ten n bits in $6
          SRU $3,$2,48          %the (b0)b1b2b3b4 in $3
          SRU $4,$2,40
	  AND $4,$4,#FF        %the s in $4
	  ANDNL $2,#1FFF        %blank out n and f in $2
	ANDNH $2,#FFFF        %blank out b1b2b3b4 in $2
	ANDNMH $2,#FF00       %blank out s in $2
	ORH	$2,#8000	%the start address of the page table in $2

%         for the moment we handle only the first immediate page
%         of the table, we assume $0 < 2^(s+10)

	  GET     $7,rJ  
	  PUSHJ   $8,newpage
	  PUT	  rJ,$7
	  ANDNH   $8,#8000     %remove sign bit to make it pysical

%         make pte and store it

          SL      $9,$1,2      %segment times 4
          SLU     $9,$3,$9     %position bi
          SRU     $9,$9,16     
	  AND     $9,$9,#0F    %mask bi
	  SL      $9,$9,13     %the starting offset for the segement table
          ADDU    $9,$2,$9     %physical start address for the segment
          SRU     $10,$0,$4    %the page number from the offset
%         at this point $10 must be smaller than 2^10
          8ADDU   $9,$10,$9    %the address of the pte


          SET     $10,#0007      %set read write and execute rights
	  8ADDU   $10,$6,$10     %add in the 10 n bits
          OR      $10,$8,$10     %add in physical address
	  STO	  $10,$9,0       %store page entry	        
	  SYNCD   7,$9,0         %make sure page table is updated in memory
          POP     0,0              


%	allocate a new page in ram and return its address
newpage	SETH	$0,#8000
	ORMH	$0,1	     %$= now points to the first byte in ram	
	LDO	$1,$0,0	     % get the pagecount
	SL	$2,$1,13     % get the pageoffset
	ADDU	$1,$1,1      % increment pagecount
	STO	$1,$0,0      % save the new page count
	ADDU	$0,$0,$2     % address of new page
	POP 1,0  	

%	  .section    .bss,"aw",@nobits

%	  .global PageCount
% RAMSTART    LOC	#8000000100000000

%PageCount OCTA 0              %First page is for OS

	