% This is a special MMIX BIOS for the Stopwatch example.
% It is considert to be loaded into the ROM
% at physical address 0000 0000 0000 0000
% used with virtual address 8000 0000 0000 0000.

%		.section    .text,"ax",@progbits	% needed for the gnu tools
		LOC	#8000000000000000
	
% page table setup (see small model in address.howto)
Main	IS	@					Keep MMIXAL happy.
Boot	GETA	$0,DTrap		Set dynamic and forced trap handler.
	    PUT	rTT,$0
	    GETA	$0,FTrap
	    PUT	rT,$0
	    GET		$0,rQ			Read rQ and
	    PUT     rQ,0				clear interrupts.
	    PUT	    rG,#F0			Make the top 16 registers global.
	   
count	IS	$254

	    SETMH	$0,#0001		Ignore the P bit.
	    NAND	$0,$0,$0
	    PUT		rK,$0			Enable interrupts.
1H	    SYNC	4				Go to power save mode
	    JMP		1B              	and loop idle.
	    


%		Entry point for a forced TRAP (not used)
FTrap	PUT		rJ,$255
		SETMH	$255,#0001		Ignore the P bit.
		NAND	$255,$255,$255
		RESUME	1

	
%		Entry point for a dynamic TRAP	
DTrap	PUSHJ	$255,DHandler
		PUT		rJ,$255
		SETMH	$255,#0001		Ignore the P bit.
		NAND	$255,$255,$255
		RESUME	1
	

DHandler   GET 	$0,rQ
	   SUBU	$1,$0,1			From xxx...xxx1000 to xxx...xxx0111
	   SADD	$2,$1,$0		Position of lowest bit
	   ANDN	$1,$0,$1		The lowest bit
       ANDN	$1,$0,$1		Delete lowest bit
	   PUT	rQ,$1				and return to rQ.
	   SLU	$2,$2,2			Scale
       GETA	$1,DTrapTable		and jump.
	   GO	$1,$1,$2


DTrapTable JMP DTrapUnhandled  %0
	       JMP DTrapUnhandled  %1
           JMP DTrapUnhandled  %2
           JMP DTrapUnhandled  %3
           JMP DTrapUnhandled  %4
           JMP DTrapUnhandled  %5
           JMP DTrapUnhandled  %6
           JMP DTrapUnhandled  %7
           JMP DTrapStart      %8
           JMP DTrapStop       %9
           JMP DTrapReset      %10
           JMP DTrapTime       %11
           JMP DTrapUnhandled  %12
           JMP DTrapUnhandled  %13
           JMP DTrapUnhandled  %14
           JMP DTrapUnhandled  %15
           JMP DTrapUnhandled  %16
           JMP DTrapUnhandled  %17
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



DTrapUnhandled	GETA	$0,2F
		SWYM	5               Inform the debugger.
		POP	0,0
2H		BYTE	"DEBUG unhandled Interrupt",0


DTrapStart	SETH	$0,#8002	Timer address
		SET		$1,100			100ms
		ORMH	$1,100
		STO		$1,$0,#10		Enable timer interrupts.
	
		POP 0,0

DTrapStop	SETH	$0,#8002	Timer address		 
		STCO	0,$0,#10		Disable timer interrupts.
		POP 	0,0

DTrapReset	SET	count,0
		GET		$0,rJ	
		PUSHJ	$1,:Display
		PUT		rJ,$0
		POP 	0,0
		
DTrapTime	ADD	count,count,1
		GET		$0,rJ	
		PUSHJ	$1,:Display
		PUT		rJ,$0
		POP 	0,0

		PREFIX	:Display:

%		Association of bits and segments
top		IS	1			
mid		IS	2
bot		IS	4
tleft	IS	8
bleft	IS	#10
tright	IS	#20
bright	IS	#40
dot		IS	#80

%		Local variables
table	IS		$0
n		IS		$1
i		IS		$2
codes	IS		$3
digit	IS		$4
code	IS		$5
addr	IS		$6	

%		Display :count on the seven segment display
:Display	GETA	table,segments			
		SET		n,:count
		SET		i,0
		SET		codes,0
1H		DIV		n,n,10				convert to decimal
		GET		digit,:rR
		LDB		code,table,digit	convert to segment bits
		SLU		code,code,i
		ADD		i,i,8
		OR		codes,codes,code
		BP		n,1B
		ORL		codes,dot<<8		add dot after first digit
		SETH	addr,#8003			sevensegment address
		STO		codes,addr,0
		POP		0,0


%		Table of seven segment codes for digits from 0 to 9
segments	BYTE	top|tleft|tright|bleft|bright|bot  	%0
		BYTE	tright|bright							%1
		BYTE	top|tright|mid|bleft|bot				%2
		BYTE	top|tright|mid|bright|bot				%3
		BYTE	tleft|tright|mid|bright					%4
		BYTE	top|tleft|mid|bright|bot				%5
		BYTE	top|tleft|mid|bleft|bright|bot			%6
		BYTE	top|tright|bright						%7
		BYTE	top|mid|bot|tleft|tright|bleft|bright	%8
		BYTE	top|mid|bot|tleft|tright|bright			%9
		