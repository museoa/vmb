	.section    .text,"ax",@progbits
	.global disk_init
	.global disk_read
	.global disk_write


diskH			IS	#8003	       high wyde of disk address	
statusOffset		IS	#00
controlOffset		IS	#04  
capacityOffset	IS     #08
sectorOffset		IS     #10 
countOffset		IS     #18 
dmaAddrOffset		IS     #20	we use only dma[0] 
dmaSizeOffset		IS	#28

		% Control Bits
DISK_GO  	IS	#01	/* a 1 written here starts the disk command */
DISK_IEN	IS	#02	/* enable disk interrupt */
DISK_WRT	IS	#04	/* command type: 0 = read, 1 = write */
 		% Status Bits 
DISK_BUSY	IS	#01	/* 1 = disk is working on a command */
	
diskInterrupt IS	19

	
disk_init	SETH	$1,diskH
		LDO	$0,$1,capacityOffset
	       ZSNZ	$0,$0,1	   return capacity != 0
		POP	1,0

	
		% parameters
sector	IS	$0
count	IS	$1
buffer	IS	$2        must be negative to be a physical address
		%local variables
base	IS	$3
control IS	$4
diskflag IS	$5	
tmp	IS	$6

		% function disk_read			
disk_read	BNN	buffer,read_false

		% write data cache
		SET	tmp,count	% make sure that the buffer content is
		SET	base,buffer   % taken to memory and removed from the cache
2H		SYNCD	#FF,base,0
		INCL	base,#100
		SYNCD	#FF,base,0
		INCL	base,#100	
		SUB	tmp,tmp,1
		BP	tmp,2B		
	
		% initialize the disk controler
		ANDNH	buffer,#8000	remove sign bit to make physical address
		SETH	base,diskH
		STO	count,base,countOffset
		STO	sector,base,sectorOffset
		STO	buffer,base,dmaAddrOffset
		SLU	count,count,9	multiply by 512=SectorSize
		STO	count,base,dmaSizeOffset

		% initialize rQ
     		SET	diskflag,1
     		SLU	diskflag,diskflag,diskInterrupt
		GET	tmp,rQ
	       ANDN	tmp,tmp,diskflag   % delete the bit
	       PUT     rQ,tmp

		% start reading
		SET	control,DISK_GO|DISK_IEN 	 read and send interrupt
		STTU	control,base,controlOffset		
		JMP	2F
		
		% wait for read to finish
1H		SYNC	4		%go to power save mode
2H		GET	tmp,rQ
	       AND     tmp+1,tmp,diskflag
		BZ	tmp+1,1B	% this was not the disk interrupt
	       ANDN	tmp,tmp,diskflag   % delete the bit
	       PUT     rQ,tmp

		LDT	control,base,statusOffset
		BN	control,read_false		test the ERROR bit

		SET	$0,1    return true
	       POP	1,0
	
read_false	SET	$0,0	return false
		POP	1,0

		% function disk_write	
disk_write	BNN	buffer,write_false
		
		% write data cache
		SET	tmp,count	% make sure that the buffer content is
		SET	base,buffer     % taken to memory and removed from the cache
2H		SYNCD	#FF,base,0
		INCL	base,#100
		SYNCD	#FF,base,0
		INCL	base,#100	
		SUB	tmp,tmp,1
		BP	tmp,2B		
	
		% initialize the disk controler
		ANDNH	buffer,#8000	remove sign bit to make physical address
		SETH	base,diskH
		STO	count,base,countOffset
		STO	sector,base,sectorOffset
		STO	buffer,base,dmaAddrOffset
		SLU	count,count,9	multiply by 512=SectorSize
		STO	count,base,dmaSizeOffset

		% initialize rQ
     		SET	diskflag,1
     		SLU	diskflag,diskflag,diskInterrupt
		GET	tmp,rQ
	       ANDN	tmp,tmp,diskflag   % delete the bit
	       PUT     rQ,tmp

		% start writing
		SET	control,DISK_GO|DISK_IEN|DISK_WRT write and send interrupt
		STTU	control,base,controlOffset		
		JMP	2F
		
		% wait for write to finish
1H		SYNC	4		%go to power save mode
2H		GET	tmp,rQ
	       AND     tmp+1,tmp,diskflag
		BZ	tmp+1,1B	% this was not the disk interrupt
	       ANDN	tmp,tmp,diskflag   % delete the bit
	       PUT     rQ,tmp
	       
	       % check status
		LDT	control,base,statusOffset
		BN	control,write_false		test the ERROR bit

		SET	$0,1    return true
	       POP	1,0		
	
write_false	SET	$0,0	return false
		POP	1,0

