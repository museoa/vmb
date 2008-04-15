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
	STO	$2,$1,8		%echo
	CMP	$2,$2,#0D	%carriage return
	BNZ	$2,1F		
	SET	$2,#0A		%line feed
	STO	$2,$1,8
1H	POP	0,0

%	read blocking a character from the keyboard
KeyboardC 	SETH	$1,console    
3H	LDO	$2,$1,0		%keyboard status/data
	BN	$2,2F	
	SR	$3,$2,32
	AND	$3,$3,#FF
	BZ	$3,2F	
	AND	$2,$2,#FF
	STO	$2,$1,8		%echo
	CMP	$3,$2,#0D	%carriage return
	BNZ	$3,1F		
	SET	$2,#0A		%line feed
	STO	$2,$1,8
1H	SET	$0,$2
	POP	1,0
%	wait
2H	SYNC	4		%go to power save mode
	JMP	3B




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
 
%	Entry point for a forced TRAP
FTrap	PUSHJ	$255,FHandler
	PUT	rJ,$255
	NEG	$255,1	  %enable interrupt $255->rK with resume 1
	RESUME	1

FHandler	GET	$0,rXX
		BNN	$0,Ropcode
		SRU	$1,$0,24       
		AND	$1,$1,#FF	%the opcode
		BZ	$1,Trap		

		POP	0,0		%not a TRAP and ropcode<0
       
Ropcode	SRU	$0,$0,56		%the ropcode
	BZ	$0,DTrapPageFault	%0 means page fault
	SUB	$1,$0,2         
	BZ      $1,Emulate		%2 means emulate the instruction
	CMP	$1,$0,3	        
	BZ	$1,Virtual	%page table translation in software

	POP     0,0             %ignore the rest


	
%       Emulate the instruction
Emulate POP     0,0		%not implemented


%	Do pagetable translation in software
Virtual SET	$0,#1230         %the dummy physical address
	PUT	rZZ,$0
	POP     0,0

%       Handle a forced Trap
Trap    GETA	$2,FTrapTable
	SRU	$1,$0,8
	AND	$1,$1,#FF		%the Y value (the function code)
        CMP     $3,$1,#1F
	BP	$3,TrapUnhandled	% in the moment we handle only very few Traps
	AND	$1,$1,#1F    
	SL	$1,$1,2
1H	GO	$2,$2,$1		%Jump into the Trap Table
	
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

TrapHalt	NEG	$0,1            %  enable interrupts
  		PUT	rK,$0
1H		SYNC	4		%go to power save mode
		JMP	1B              % and loop idle

TrapFputs 	AND     $0,$0,#0FF    %get the Z value 
        	BZ      $0,1F     %this is stdin
        	CMP     $1,$0,2
        	BNP     $1,4F     %this is stdout or stderr
%       	this is a file 
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


%	Utilities

%       memcpy utility: copy $2 bytes from $1 to $0
1H	LDBU	$3,$1,0  
	STBU    $3,$0,0
        ADD	$0,$0,1
	ADD	$1,$1,1
	SUB	$2,$2,1
memcpy	BP	$2,1B
	POP	0,0

%       octacpy utility: like memcpy but with a multiple of octas
1H	LDOU	$3,$1,0  %copy $2 octas from $1 to $0
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
	
TrapFopen POP	0,0

TrapFclose POP	0,0

TrapFread POP	0,0

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


TrapFgetws POP	0,0




TrapFwrite AND     $0,$0,#0FF    %get the Z value 
        BZ      $0,1F     %this is stdin
        CMP     $1,$0,2
        BNP     $1,4F     %this is stdout or stderr
%       this is a file 
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


TrapFseek POP	0,0

TrapFtell  POP	0,0


%		Put one pixel on the graphics display. 
%		In $255 we havein the Hi 32 bit the offset
%               and in the low 24 bit the RGB value 
TrapGPutPixel GET	$0,rBB		%get the $255 parameter: address and RGB
	      SRU       $1,$0,32	%offset
              SETH      $2,#8002	%base address of vram
              STTU      $0,$2,$1
              PUT	rBB,0		%the result is returned with resume 1
	      POP	0,0


TrapUnhandled	SWYM	5		% tell the debugger
		POP	0,0

%	Put one character contained in $0 on the screen
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
	BN	$5,1F		%Still space in the Buffer
	SYNC	4		%wait idle for a screen interrupt 
	JMP	2B		%telling that the screen may again accept characters

1H	ADD	$2,$1,16	%address of screen buffer
	STB	$0,$2,$3
	ADD	$3,$3,1
	AND	$3,$3,#FF
	STTU	$3,$1,8
	POP	0,0


%       The ROM Page Table

	LOC	#8000000000002000	%The start is fixed in mmix-sim.ch
PageTab OCTA	#0000000100002007	%text, should be ...001 for execute only
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
   	 
	LOC     (@&~#1FFF)+#2000	%data
	OCTA	#000000010001a006  
	OCTA	#000000010001c006  
	OCTA	#000000010001e006  
	OCTA	#0000000100020006  
	OCTA	#0000000100022006  
	OCTA	#0000000100024006  
	OCTA	#0000000100026006  
	OCTA	#0000000100028006  

	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010002a006	%pool
	OCTA	#000000010002c006  
	
	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010002e006	%stack
	OCTA	#0000000100030006  

	LOC	(@&~#1FFF)+#2000-2*8	
	OCTA	#0000000100032006       %gcc memory stack < #6000000000800000
	OCTA	#0000000100034006  

	LOC	(@&~#1FFF)+#2000
	
%       the table maps each segement with up to 1024 pages
%	currently, the first page is system rom, the next four pages are for
%       text, data, pool, and stack.
%       The page tables imply the following RAM Layout

%	The RAM Layout

%       ram layout the small memmory model (see memory.howto)
%       8000000100000000    first page for OS, layout see below
%       Next the following pages for the user programm
%       Text Segment 10 pages = 80kByte
%       8000000100002000    text segment 0
%       8000000100004000    text segment 1
%       8000000100006000    text segment 2
%       8000000100008000    text segment 3
%       800000010000a000    text segment 4
%       800000010000c000    text segment 5
%       800000010000e000    text segment 6
%       8000000100010000    text segment 7
%       8000000100012000    text segment 8
%       8000000100014000    text segment 9
%       Data Segment 10 pages = 80 kByte
%       8000000100016000    data segment 0
%       8000000100018000    data segment 1
%       800000010001a000    data segment 2
%       800000010001c000    data segment 3
%       800000010001e000    data segment 4
%       8000000100020000    data segment 5
%       8000000100022000    data segment 6
%       8000000100024000    data segment 7
%       8000000100026000    data segment 8
%       8000000100028000    data segment 9
%	Pool Segment 2 pages = 16 kByte
%       800000010002a000    pool segment 0
%       800000010002c000    pool segment 1
%	Stack Segment 2 pages = 16 kByte
%       800000010002e000    stack segment 0
%       8000000100030000    stack segment 1

%       free space starts at 8000000100032000

%       initialize the memory management
memory	SETH    $0,#1234	%set rV register
	ORMH    $0,#0D00      
	ORML    $0,#0000
	ORL     $0,#2000
	PUT	rV,$0

	SETH    $0,#8000       %$0 physical address of RAM
        ORMH    $0,#0001 
        
        SET	$1,$0
	ORML	$1,#0003
        ORL     $1,#8000       %address of first empty page
	STO	$1,$0,0        %initialize FreeSpace
        SET	$1,0
        STO	$1,$0,#8	%initialize ScreenBufferStart 
        STO	$1,$0,#10	%initialize ScreenBufferEnd
	POP     0,0

%       TRAP handler for page faults (not yet implemented)       	
DTrapPageFault	POP     0,0              

%	allocate a new page in ram and return its address
newpage	SETH	$1,#8000
	ORMH	$1,#0001	%$= now points to the first byte in ram	
	LDO	$0,$1,0		% get the FreeSpace
	SET	$2,$0
	INCL	$2,#2000	% add one page
	STO	$2,$1,0		% save the new FreeSpace
	POP 1,0  	


%	First Page in RAM: reserved for the OS.
%	The layout follows below.
%	  .section	.bss,"aw",@nobits
%	  .global	FreeSpace
% RAMSTART    LOC	#8000000100000000

%FreeSpace		OCTA 0              %First page is for OS
%ScreenBufferStart	OCTA 0
%ScreenBufferEnd	OCTA 0
%ScreenBuffer		BYTE 0      255 Byte of screen Buffer
%			LOC	@+#FF
	