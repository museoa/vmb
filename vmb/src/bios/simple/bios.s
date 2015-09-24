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
   	OCTA	#000000010001a007 
   	OCTA	#000000010001c007 
   	OCTA	#000000010001e007 
   	OCTA	#0000000100020007
   	OCTA	#0000000100022007
   	OCTA	#0000000100024007
	OCTA	#0000000100026007 
   	OCTA	#0000000100028007 
   	OCTA	#000000010002a007 
   	OCTA	#000000010002c007 
   	OCTA	#000000010002e007 
   	OCTA	#0000000100030007
   	OCTA	#0000000100032007
   	OCTA	#0000000100034007
	OCTA	#0000000100036007 
   	OCTA	#0000000100038007 
   	 
%       Data Segment 8 pages = 80 kByte
	LOC     (@&~#1FFF)+#2000	%data
	OCTA	#000000010003a006  
	OCTA	#000000010003c006  
	OCTA	#000000010003e006  
	OCTA	#0000000100040006  
	OCTA	#0000000100042006  
	OCTA	#0000000100044006  
	OCTA	#0000000100046006  
	OCTA	#0000000100048006  
	OCTA	#000000010004a006  
	OCTA	#000000010004c006  
	OCTA	#000000010004e006  
	OCTA	#0000000100050006  
	OCTA	#0000000100052006  
	OCTA	#0000000100054006  
	OCTA	#0000000100056006  
	OCTA	#0000000100058006  

%	Pool Segment 2 pages = 16 kByte
	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010005a006	%pool
	OCTA	#000000010005c006  
	OCTA	#000000010005e006  
	OCTA	#0000000100060006  
	OCTA	#0000000100062006  
	OCTA	#0000000100064006  
	OCTA	#0000000100066006  
	OCTA	#0000000100068006  
	OCTA	#000000010006a006
	OCTA	#000000010006c006  
	OCTA	#000000010006e006  
	OCTA	#0000000100070006  
	OCTA	#0000000100072006  
	OCTA	#0000000100074006  
	OCTA	#0000000100076006  
	OCTA	#0000000100078006  
	OCTA	#000000010007a006
	OCTA	#000000010007c006  
	OCTA	#000000010007e006  
	OCTA	#0000000100080006  
	OCTA	#0000000100082006  
	OCTA	#0000000100084006  
	OCTA	#0000000100086006  
	OCTA	#000000010008006  
	OCTA	#000000010008a006
	OCTA	#000000010008c006  
	OCTA	#000000010008e006  
	OCTA	#0000000100090006  
	OCTA	#0000000100092006  
	OCTA	#0000000100094006  
	OCTA	#0000000100096006  
	OCTA	#0000000100098006  
	
%	Stack Segment 2+2 pages = 32 kByte
	LOC	(@&~#1FFF)+#2000
	OCTA	#000000010009a006	%stack
	OCTA	#000000010009c006  
	OCTA	#000000010009e006  
	OCTA	#00000001000a0006  
	OCTA	#00000001000a2006  
	OCTA	#00000001000a4006  
	OCTA	#00000001000a6006  
	OCTA	#00000001000a8006  

	LOC	(@&~#1FFF)+#2000-2*8	
	OCTA	#00000001000aa006       %gcc memory stack < #6000000000800000
	OCTA	#00000001000ac006  

	LOC	(@&~#1FFF)+#2000

PastAllocated IS #80000001000ae000  % here FreeSpace will start		


%	First Page in RAM: reserved for the OS.
%	The layout follows below.
	  .section	.bss,"aw",@nobits



FreeSpace		OCTA 0              %First page is for OS
