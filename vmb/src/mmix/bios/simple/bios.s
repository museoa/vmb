%	this is the MMIX BIOS
%	it is considert to be the ROM loaded 
%	at physical address 0000 0000 0000 0000
%	used with 
%	virtual address 8000 0000 0000 0000

	.section    .text,"ax",@progbits
	.global PageTab		
%	LOC	#8000000000000000
	
% page table setup (see small model in address.howto)

Boot	GETA	$0,DTrap	%set dynamic- and forced-trap  handler
	PUT	rTT,$0
	GETA	$0,FTrap
	PUT	rT,$0
	PUSHJ	$0,initMemory	%initialize the memory setup
        PUSHJ	$0,initTerminal
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


	
%	Entry point for a dynamic TRAP	
DTrap	PUSHJ	$255,DHandler
	PUT	rJ,$255
	NEG	$255,1		% enable interrupt $255->rK with resume 1
	RESUME	1
	


	
%	Entry point for a forced TRAP
FTrap	PUSHJ	$255,FHandler
	PUT	rJ,$255
	NEG	$255,1	  %enable interrupt $255->rK with resume 1
	RESUME	1


%       The ROM Page Table
%       the table maps each segement with up to 1024 pages
%	currently, the first page is system rom, the next four pages are for
%       text, data, pool, and stack. then there is mor bios code.
%       The page tables imply the following RAM Layout

%	The RAM Layout

%       the ram layout uses the small memmory model (see memory.howto)
%       8000000100000000    first page for OS, layout see below
%       Next the  pages for the user programm


	
%       initialize the memory management
initMemory	SETH    $0,#1234	%set rV register
		ORMH    $0,#0D00      
		ORML    $0,#0000
		ORL     $0,#2000
		PUT	rV,$0


		GETA	$0,FreeSpace
		GETA	$1,PastAllocated
		STO	$1,$0,0         %initialize FreeSpace
	        SET	$1,0
	        STO	$1,$0,#8	%initialize ScreenBufferStart 
	        STO	$1,$0,#10	%initialize ScreenBufferEnd
		POP     0,0

%       TRAP handler for page faults (not yet implemented)       	
DTrapPageFault	POP     0,0              

%	allocate a new page in ram and return its address
newpage	GETA	$1,FreeSpace
	LDO	$0,$1,0		% get the FreeSpace
	SET	$2,$0
	INCL	$2,#2000	% add one page
	STO	$2,$1,0		% save the new FreeSpace
	POP 1,0  	



        .org #2000              % the position of the page table at 8000000000002000
	                        % is used in mmix-sim.ch to initioalize rV
	                        % to be able to load mmo files from the simulator
	
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

PastAllocated IS #8000000100036000  % here FreeSpace will start		


%	First Page in RAM: reserved for the OS.
%	The layout follows below.
	  .section	.bss,"aw",@nobits



FreeSpace		OCTA 0              %First page is for OS
