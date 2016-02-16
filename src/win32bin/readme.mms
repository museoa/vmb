%	this is a miniature MMIX BIOS
%	it is considert to be the ROM loaded 
%	at physical address 0000 0000 0000 0000
%	used with 
%	virtual address 8000 0000 0000 0000
%      it copies the keyboard to the screen

		.section    .text,"ax",@progbits		
		LOC	#8000000000000000

IOBaseHI   		IS	#8001
keyboard   		IS 0
screen	    	IS 8
	
%	read blocking a character from the keyboard and echo
IO	    	IS	$255
c			IS	$0
tmp			IS	$1


Main 		SETH	IO,IOBaseHI
slowdown	SYNC	4		%go to power save mode
loop		LDOU	c,IO,keyboard		%keyboard status/data
			BN	c,loop	
			SR	tmp,c,32
			AND	tmp,tmp,#FF		%count
			BZ	tmp,slowdown	%no character available	
			AND	tmp+1,c,#FF
			PUSHJ	tmp,output
			JMP	loop
		
wait		SYNC	4
output		LDOU	tmp,IO,screen
			BNZ	tmp,wait
			STOU	c,IO,screen
			POP	0,0

		LOC	#8000000000002000	%The start is fixed in mmix-sim.ch
								%To allow loading mmo files from the commandline

%		empty page tables
%       Text Segment 12 pages = 96kByte
Table	OCTA	0	%text permission 5=r-x
   	 
%       Data Segment 8 pages = 64 kByte RAM
		LOC     (@&~#1FFF)+#2000	%data permission rw-
		OCTA	 0
				
%		Pool Segment 2 pages = 16 kByte
		LOC	(@&~#1FFF)+#2000
		OCTA	0		
%		Stack Segment 10+2 pages = 80+16 kByte
		LOC	(@&~#1FFF)+#2000
		OCTA	0	%10 pages register stack

		LOC	(@&~#1FFF)+#2000
		OCTA	0	%10 pages register stack
