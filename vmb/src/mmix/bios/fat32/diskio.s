	.section    .text,"ax",@progbits
	.global disk_init
	.global disk_read
	.global disk_write


diskH		IS	#8003	       high wyde of disk address	
controlOffset	IS	#00  
countOffset	IS      #08 
sectorOffset	IS      #10 
dmaOffset	IS      #18 
capacityOffset	IS      #20
	

	
disk_init	SETH	$1,diskH
		LDO	$0,$1,capacityOffset
	        ZSNZ	$0,$0,1	   return capacity != 0
		POP	1,0

	
		% parameters
sector	IS	$0
count	IS	$1
buffer	IS	$2        should be negative to be a physical address
		%local variables
base	IS	$3
control IS	$4
diskflag IS	$5	
tmp	IS	$6
					
disk_read	BNN	buffer,1F

		SET	tmp,count	% make sure that the buffer content is
		SET	base,buffer     % taken to memory and removed from the cache
2H		SYNCD	#FF,base,0
		INCL	base,#100
		SYNCD	#FF,base,0
		INCL	base,#100	
		SUB	tmp,tmp,1
		BP	tmp,2B		
	
		ANDNH	buffer,#8000	remove sign bit to make physical address
		SETH	base,diskH
		STO	buffer,base,dmaOffset
		STO	count,base,countOffset
		STO	sector,base,sectorOffset
		SET	control,#3    IEN|STRT   to make it read
		STO	control,base,controlOffset
3H		SYNC	4		%go to power save mode
		GET	tmp,rQ
     		SETML	diskflag,0x0008
	        AND     diskflag,diskflag,tmp
		BZ	diskflag,3B	% this was not the disk interrupt
	        ANDN	tmp,tmp,diskflag   % delete the bit
	        PUT     rQ,tmp

2H		LDO	control,base,controlOffset load controll
		AND	tmp,control,#10	test the BUSSY bit
		BNZ	tmp,2B
		AND	tmp,control,#08	test the ERROR bit
	        BNZ	tmp,1F	


		SET	$0,1    return true
	        POP	1,0
	
1H		SET	$0,0	return false
		POP	1,0
	
disk_write	BNN	buffer,1F

		SET	tmp,count	% make sure that the buffer content is
		SET	base,buffer     % taken to memory and removed from the cache
2H		SYNCD	#FF,base,0
		INCL	base,#100
		SYNCD	#FF,base,0
		INCL	base,#100	
		SUB	tmp,tmp,1
		BP	tmp,2B		
	
		ANDNH	buffer,#8000	remove sign bit to make physical address
		SETH	base,diskH
		STO	buffer,base,dmaOffset
		STO	count,base,countOffset
		STO	sector,base,sectorOffset
		SET	control,#7    WRITE|IEN|STRT   to make it write
		STO	control,base,controlOffset
		SYNC	4		%go to power save mode

2H		LDO	control,base,controlOffset load controll
		AND	tmp,control,#10	test the BUSSY bit
		BNZ	tmp,2B
		AND	tmp,control,#08	test the ERROR bit
	        ZSZ	$0,tmp,1	   return  ERROR != 1
		POP	1,0
		
	
1H		SET	$0,0	return false
		POP	1,0

