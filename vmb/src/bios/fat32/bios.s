%	this is the MMIX BIOS
%	it is considert to be the ROM loaded 
%	at physical address 0000 0000 0000 0000
%	used with 
%	virtual address 8000 0000 0000 0000

	.section    .text,"ax",@progbits
	.global Boot	
%	LOC	#8000000000000000

Boot	GETA	$0,DTrap	%set dynamic- and forced-trap  handler
	PUT	rTT,$0
	GETA	$0,FTrap
	PUT	rT,$0
	SET	$0,#e0
	PUT	rG,$0               % allocate 32 global registers for gcc
	GETA	$254,OSStackStart
	SET	$253,0              % the frame pointer for gcc
	PUSHJ	$0,initMemory	%initialize the memory setup
        PUSHJ	$0,initTerminal
	PUSHJ	$0,fat32_initialize
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

	
%	Entry point for a dynamic TRAP	
DTrap	PUSHJ	$255,1F		% save local registers
	PUT	rJ,$255
	NEG	$255,1		% enable interrupt $255->rK with resume 1
	RESUME	1

1H	SET 	$0,$255		%save global registers
	SET	$1,$254
	SET	$2,$253
	GET 	$3,rJ
	PUSHJ	$4,DHandler
	PUT	rJ,$3
	SET     $253,$2
	SET	$254,$1
	SET	$255,$0
	POP	0,0

	
%	Entry point for a forced TRAP
FTrap	PUSHJ	$255,1F
	PUT	rJ,$255
	NEG	$255,1	  %enable interrupt $255->rK with resume 1
	RESUME	1
1H	SET 	$0,$255		%save global registers
	SET	$1,$254
	SET	$2,$253
	GET 	$3,rJ
	PUSHJ	$4,FHandler
	PUT	rJ,$3
	SET     $253,$2
	SET	$254,$1
	SET	$255,$0
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


	
%       initialize the memory management
initMemory	SETH    $0,#1234	%set rV register
		ORMH    $0,#0D00      
		ORML    $0,#0000
		ORL     $0,#2000
		PUT	rV,$0


		GETA	$0,FreeSpace
		GETA	$1,__Ebss	% end of the statically allocated RAM
	        SET	$2,#1FFF
	        ADD	$1,$1,$2
		ANDN	$1,$1,$2        % round to next multiple of #2000
		STO	$1,$0,0         % initialize FreeSpace
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


FirstUserPage IS 0x2000	%nedded for the pagetable 

	.include "pagetab.s"   % defines UserRamSize
	
%	First Pages in RAM are for the OS.
	
	  .section	.bss,"aw",@nobits
	.org	0
OSRam	     IS	  @
FreeSpace    OCTA 	0

	.org	FirstUserPage
	OCTA	0

	.org	FirstUserPage+UserRamSize
	OCTA	0


OSStackSize	IS	16*0x2000 % what we need for the gcc memory stack

	.org	FirstUserPage+UserRamSize+OSStackSize
	.global OSStackStart
OSStackStart	IS	@
	OCTA	0	% first OCTA after preallocated space, it follows OS RAM until __Ebss

