%	this is a simple MMIX BIOS
%	it is considert to be the ROM loaded 
%	at        physical address 0000 0000 0000 0000
%	used with virtual  address 8000 0000 0000 0000

	.section    .text,"ax",@progbits		
	LOC	#8000000000000000
	
% page table setup (see small model in address.howto)
Main	IS	@  dummy		Main, to keep mmixal happy
Boot	GETA	$0,DTrap	set dynamic- and forced-trap  handler
		PUT		rTT,$0	
		GETA	$0,FTrap
		PUT		rT,$0
		PUSHJ	$0,Memory	initialize the memory setup
        SET		$0,0
		PUT     rQ,$0		clear interrupts

%		here we start a loaded user program
%       rXX should be #FB0000FF = UNSAVE $255
%		rBB is coppied to $255, it should be the place in the stack 
%		where UNSAVE will find its data
%		rWW should be the entry point in the main program, 
%		thats where the program
%		continues after the UNSAVE.
%		If no program is loaded, rXX will be 0, that is TRAP 0,Halt,0
%		and we end the program before it has started in the Trap handler.
	
		NEG		$255,1		enable interrupt $255->rK with resume 1
		RESUME	1			loading a file sets up special registers for that

	
DTrap	PUSHJ	$255,DHandler
		PUT		rJ,$255
		NEG		$255,1		enable interrupt $255->rK with resume 1
		RESUME	1
	
DHandler GET 	$0,rQ
		SUBU	$1,$0,1		from xxx...xxx1000 to xxx...xxx0111
		SADD	$2,$1,$0	position of lowest bit
		ANDN	$1,$0,$1	the lowest bit
        ANDN	$1,$0,$1	delete lowest bit
		PUT		rQ,$1		and return to rQ
		SLU		$2,$2,2		scale
        GETA	$1,DTrapTable	and jump
		GO		$1,$1,$2


DTrapTable JMP DTrapUnhandled  %0
           JMP DTrapUnhandled  %1
           JMP DTrapUnhandled  %2
           JMP DTrapUnhandled  %3
           JMP DTrapUnhandled  %4
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
           JMP DTrapKey        %17
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


IO   	IS		#8001             %   hi wyde of IO	

DTrapKey	SETH	$1,IO    
			LDO		$2,$1,0		keyboard status/data
			BN		$2,1F	
			SR		$3,$2,32
			AND		$3,$3,#FF
			BZ		$3,1F	
			AND		$2,$2,#FF
			STO		$2,$1,8		echo
			CMP		$2,$2,#0D	carriage return
			BNZ		$2,1F		
			SET		$2,#0A		line feed
			STO		$2,$1,8
1H			POP		0,0

DTrapScreen		POP	0,0					ignore

DTrapUnhandled	GETA	$0,1F
				SWYM	$0,5            inform the debugger
				POP	0,0
1H				BYTE	"DEBUG Unhandled Exception",0
 
%			Entry point for a forced TRAP
FTrap		PUSHJ	$255,FHandler
			PUT	rJ,$255
			NEG	$255,1	  %enable interrupt $255->rK with resume 1
			RESUME	1

FHandler	GET		$0,rXX
			BNN		$0,1F				ropcode < 0 ?
			SRU		$1,$0,24       
			AND		$1,$1,#FF			The opcode
			BNZ		$1,1F				opcode = TRAP ? 
			GETA	$2,FTrapTable		Handle a forced Trap.
			SRU		$1,$0,8
			AND		$1,$1,#FF			the Y value (the function code)
			CMPU    $3,$1,#0F
			BP		$3,TrapUnhandled	In the moment we handle only a few Traps.
			SL		$1,$1,2
			GO		$2,$2,$1			Jump into the Trap Table
1H	  		POP	0,0						Done

FTrapTable 	JMP   TrapHalt  %0
			JMP   TrapUnhandled  %1
			JMP   TrapUnhandled  %2
			JMP   TrapUnhandled  %3
			JMP   TrapUnhandled  %4
			JMP   TrapUnhandled  %5
			JMP   TrapUnhandled  %6
			JMP   TrapFputs      %7 
			JMP   TrapUnhandled  %8
			JMP   TrapUnhandled  %9
			JMP   TrapUnhandled  %a
			JMP   TrapUnhandled  %b
			JMP   TrapUnhandled  %c
			JMP   TrapUnhandled  %d
			JMP   TrapUnhandled  %e
			JMP   TrapUnhandled  %f

%         The individual Trap routines

TrapHalt	GETA	$0,2F
			SWYM	$0,5               	   	inform the debugger
1H			SYNC	4						go to power save mode
			GET		$0,rQ					check for interrupts
			BZ		$0,1B
			PUSHJ	$0,DHandler
			JMP		1B              		and loop
2H			BYTE	"DEBUG Program halted",0


TrapFputs 	AND     $0,$0,#0FF    	get the Z value 
        	BZ      $0,1F     	  	this is stdin
        	CMP     $1,$0,2
        	BP		$1,1F     		
%       	Fputs to the screen
			GET		$0,rBB    		get the $255 parameter
			SETH	$1,IO	
			JMP 	2F
3H			STO		$3,$1,8			direct output
        	ADD		$0,$0,1
2H			LDB		$3,$0,0
        	BNZ     $3,3B
1H			POP	0,0

TrapUnhandled	GETA	$0,1F
				SWYM	$0,5               inform the debugger
				POP		0,0
1H				BYTE	"DEBUG Unhandled TRAP",0


%       The ROM Page Table
%       the table maps each segement with up to 1024 pages
%	    currently, the first page is system rom, the next four pages are for
%       text, data, pool, and stack. then there is mor bios code.
%       The page tables imply the following RAM Layout

%	    The RAM Layout

%       the ram layout uses the small memmory model (see memory.howto)
%       8000000100000000    first page for OS, layout see below
%       Next the  pages for the user programm


%       free space starts at 8000000100032000
	
%       initialize the memory management
Memory	SETH    $0,#1234	set rV register
		ORMH    $0,#0D00      
		ORML    $0,#0000
		ORL     $0,#2000
		PUT		rV,$0
		POP     0,0

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

%		Pool Segment 2 pages = 16 kByte
		LOC	(@&~#1FFF)+#2000
		OCTA	#000000010002a006	%pool
		OCTA	#000000010002c006  
	
%		Stack Segment 2+2 pages = 32 kByte
		LOC	(@&~#1FFF)+#2000
		OCTA	#000000010002e006	%stack
		OCTA	#0000000100030006  

		LOC	(@&~#1FFF)+#2000-2*8	
		OCTA	#0000000100032006       %gcc memory stack < #6000000000800000
		OCTA	#0000000100034006  

		LOC	(@&~#1FFF)+#2000
