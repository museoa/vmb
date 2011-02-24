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
           JMP DTrapTimer      %11
           JMP DTrapStep       %12
           JMP DTrapStart      %13
           JMP DTrapStop       %14
           JMP DTrapHalt       %15
           JMP DTrapUnhandled  %16
           JMP DTrapUnhandled  517
           JMP DTrapUnhandled  %18
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



DTrapStart	SETH	$0,#8003	% timer address
		SET	$1,1000		%1000 ms
		STO	$1,$0,0		%enable timer interrupts
		POP 0,0

DTrapStop	SETH	$0,#8003	% timer address
		SET	$1,0		% 
		STO	$1,$0,0		%disable timer interrupts
		POP 0,0
DTrapTimer	SETH	$1,#8000
		ORMH	$1,#0001	%$1 now points to the first byte in ram	
		SET	$0,1
		STO	$0,$1,16	% third octa is the step flag
		POP	0,0

DTrapStep	SETH	$1,#8000
		ORMH	$1,#0001	%$1 now points to the first byte in ram	
		SET	$0,1
		STO	$0,$1,16	% third octa is the step flag
		POP	0,0

DTrapHalt	SETH	$1,#8000
		ORMH	$1,#0001	%$1 now points to the first byte in ram	
		SET	$0,1
		STO	$0,$1,8		% second octa is the terminate flag
		POP	0,0
				
DTrapUnhandled	GETA	$0,1F
		SWYM	$0,5               % inform the debugger
		POP	0,0
1H		BYTE	"DEBUG Unhandled Interrupt",0


		
 
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
	  JMP   TrapUnhandled  %1
	  JMP   TrapUnhandled  %2
	  JMP   TrapUnhandled  %3
	  JMP   TrapUnhandled  %4
	  JMP   TrapUnhandled  %5
	  JMP   TrapUnhandled  %6
	  JMP   TrapUnhandled  %7 
	  JMP   TrapUnhandled  %8
	  JMP   TrapUnhandled  %9
	  JMP   TrapUnhandled  %a
	  JMP   TrapUnhandled  %b
	  JMP   TrapUnhandled  %c
	  JMP   TrapUnhandled  %d
	  JMP   TrapUnhandled  %e
	  JMP   TrapUnhandled  %f
	  JMP   TrapGPutPixel %10
	  JMP   TrapTWait     %11
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

TrapHalt	GETA	$0,2F
		SWYM	$0,5		% tell the debugger
		NEG	$0,1            % enable interrupts
  		PUT	rK,$0
1H		SYNC	4		% go to power save mode
		JMP	1B              % and loop idle
2H		BYTE	"DEBUG Program terminated",0		



%		Put one pixel on the graphics display. 
%		In $255 we havein the Hi 32 bit the offset
%               and in the low 24 bit the RGB value 
TrapGPutPixel GET	$0,rBB		%get the $255 parameter: address and RGB
	      SRU       $1,$0,32	%offset
              SETH      $2,#8002	%base address of vram
              STTU      $0,$2,$1
              PUT	rBB,0		%the result is returned with resume 1
	      POP	0,0


1H		SYNC	4		% go to power save mode
		GET	$0,rJ
		PUSHJ	$1,DHandler     % handle interrupts
		PUT	rJ,$0
TrapTWait	SETH	$1,#8000
		ORMH	$1,#0001	%$= now points to the first byte in ram	
		LDO	$0,$1,8		% second octa is our terminate flag
		BNZ	$0,2F
		LDO	$0,$1,16	% third octa is the step flag
		BZ	$0,1B		% wait more
		SET	$0,0
		STO	$0,$1,16	% reset step flag
		PUT	rBB,1		%the result 1 is returned with resume 1	
		POP	0,0

2H		GET	$0,rJ
		PUSHJ	$1,DTrapStop
		PUT	rJ,$0
		PUT	rBB,0		%the result 0 is returned with resume 1	
		POP	0,0


TrapUnhandled	GETA	$0,1F
		SWYM	$0,5		% tell the debugger
		POP	0,0
1H		BYTE	"DEBUG Unhandled TRAP",0		


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
%       Text Segment 10 pages = 80kByte
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
   	 
%       Data Segment 8 pages = 80 kByte
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

	LOC	(@&~#1FFF)+#2000-2*8	
	OCTA	#0000000100032006       %gcc memory stack < #6000000000800000
	OCTA	#0000000100034006  

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
	ORML	$1,#0003
        ORL     $1,#8000       %address of first empty page
	STO	$1,$0,0        %initialize FreeSpace
        SET	$1,0
        STO	$1,$0,#8	%initialize ScreenBufferStart 
        STO	$1,$0,#10	%initialize ScreenBufferEnd
	POP     0,0

%       TRAP handler for page faults (not yet implemented)       	
DTrapPageFault	GETA	$0,1F
		SWYM	$0,5		% tell the debugger
		POP	0,0
1H		BYTE	"DEBUG Unhandled Page Fault",0	
       

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
	