
%	First Page in RAM: reserved for the OS.
%	The layout follows below.

	.section	.bss,"aw",@nobits

		
ScreenBufferStart	OCTA 0
ScreenBufferEnd	        OCTA 0
ScreenBuffer		BYTE 0      255 Byte of screen Buffer
			LOC	@+#FF

	.section    .text,"ax",@progbits

	.global initTerminal
	.global KeyboardC 
	.global DTrapKey
	.global ScreenC
	.global DTrapScreen
	
	
initTerminal	GETA	$0,ScreenBufferStart
		SET	$1,0
	        STO	$1,$0,0		%initialize ScreenBufferStart 
		STO	$1,$0,#8	%initialize ScreenBufferEnd
		POP     0,0
	

console	IS	#8001             %   hi wyde of console	

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
	BNN	$2,4F
	%	wait
2H	SYNC	4		%go to power save mode
	JMP	3B

4H	SR	$3,$2,32
	AND	$3,$3,#FF
	BZ	$3,2B
	AND	$2,$2,#FF
	STO	$2,$1,8		%echo
	CMP	$3,$2,#0D	%carriage return
	BNZ	$3,1F		
	SET	$2,#0A		%line feed
	STO	$2,$1,8
1H	SET	$0,$2
	POP	1,0



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

