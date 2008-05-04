 #$Id: tapeboot.s,v 1.1 2008-05-04 15:47:00 mbbh Exp $

	.abicalls

 # GCC automatic generated header
 
 # GNU C 2.7.2 [AL 1.1, MM 40] Linux/MIPSEL compiled by GNU C

 # Cc1 defaults:
 # -mgas -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -version -o
 # 

 	# Constants

	STATUSMASK 	= 0xFDBFFFF3
	BIOSVECTPAGE 	= 0x20000000
	BIOSPANIC	= 0x00000088
	VECTAREANUM	= 8
	VECTPCPOS	= 12
	VECTSIZE	= 140
	PROGVSTART	= 0x20001004

	TERM0COMMAND	= 0x1000025C
	BYTELEN 	= 8
	PRINTCHR	= 2	
	DOTCHAR		= 46
	LFCHAR		= 10

	TAPE0BASE	= 0x100000D0
	TAPESTART	= 3
	READBLK		= 3
	
	ACK		= 1
	BUSYCODE	= 3
	READYCODE	= 1
	EOBCODE		= 2

	RAMBASE		= 0x20000000
	BLOCKSIZE	= 4096

	PADDINGTLB	= 0x100 - 8
	PADDINGEXC	= 0x80 - 16

	#Code start

	.text
	.align	2
	.globl	tapeboot
	.type	tapeboot,@function
	.ent	tapeboot

tapeboot:
	.frame	$fp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set noat

	.set noreorder
	.set nomacro

	# boot starts here 
	j LTapeBootStart
	nop

	.space PADDINGTLB

	# TLB miss should not happen during boot phase: jumps to BIOS PANIC
	lui	$26, 0x0000
	ori	$26, $26, 0x88
	jr	$26
	nop

	.space PADDINGEXC

	# no other exceptions should happen during boot phase:
	# jumps to BIOS PANIC
	lui	$26, 0x0000
	ori	$26, $26, 0x88
	jr	$26
	nop	

	.set reorder
	.set macro

LTapeBootStart:	
	# mapping all exception vectors to BIOS code
	# this is done resetting bit 22 in STATUS CP0 register
	# we also assure that kernel will start in kernel mode, with
	# interrupts and VM disabled: this is done zeroing bits 3, 2 and 25
	mfc0	$4, $12
	li	$5, STATUSMASK   
	and	$4, $4, $5
	mtc0	$4, $12

	# loads the kernel from tape: for each block loaded a dot will be
	# printed on terminal 0 

	# initializations	

	li 	$2, TAPE0BASE
	li 	$3, TAPESTART
	lw	$4, +12($2)
	
	# if tape is not loaded from start, BIOS panic() routine is called
	beq	$3, $4, LStartLoad
	
	li	$26, BIOSPANIC
	jr	$26

LStartLoad:	

	# else loading may start

	# physical memory load index
	li	$26, RAMBASE - BLOCKSIZE
	
	# tape READBLK command
	li	$27, READBLK

	# terminal 0 COMMAND address and busy code
	li 	$7, TERM0COMMAND
	li 	$8, BUSYCODE

	# tape loading main loop 

LTapeLoadLoop:
	# stores physical loading address into TAPE0 DATA0 register
	# and starts read operation

	addi 	$26, BLOCKSIZE

	sw 	$26, +8($2)
	sw 	$27, +4($2)
	
	li	$3, BUSYCODE

LTapeWaitLoop:
	#loads STATUS register
	lw	$4, +0($2)
	beq	$3, $4, LTapeWaitLoop

	# tape STATUS ($4) is no more BUSY: looking for errors
	li 	$3, READYCODE

	beq	$4, $3, LContinueLoad

	li	$26, BIOSPANIC
	jr	$26

LContinueLoad:
	# all ok

LPrintDotWaitLoop:

	# awaits terminal 0 availability

	lw 	$6, -4($7)
	beq	$6, $8, LPrintDotWaitLoop
	
	#load ASCII value of '.' into register 
	li	$5, DOTCHAR

	#prepares PRINTCHAR command
	sll 	$5, BYTELEN
	addi	$5, PRINTCHR
	sw	$5, 0($7)


	# examines tape DATA1 register to see if next block must be read:
	# load ends when an EOF or EOT is found, thus it continues when
	# an EOB is found

	lw 	$4, +12($2)
	li	$3, EOBCODE

	beq	$3, $4, LTapeLoadLoop

	#tape load ends here: clearing tape interrupt

	li 	$27, ACK	
	sw	$27, +4($2)


LPrintLFWaitLoop:

	# awaits terminal 0 availability

	lw 	$6, -4($7)
	beq	$6, $8, LPrintLFWaitLoop
	
	#load ASCII value of LF into register 
	li	$5, LFCHAR

	#prepares PRINTCHAR command
	sll 	$5, BYTELEN
	addi	$5, PRINTCHR
	sw	$5, 0($7)

	# awaits terminal 0 availability again

LPrintLoop2:
	lw 	$6, -4($7)
	beq	$6, $8, LPrintLoop2

	# clears pending interrupt on terminal 0

	li	$5, ACK
	sw	$5, 0($7)
	
	# setting boot exception vectors into first page of RAM memory:
 	# at first, all exceptions cause a kernel panic()

	# load kernel panic() BIOS routine address into every PC field
	# of exception vectors

	# there are: 1 vector for interrupt, 1 for TLB memory management, 
	# 1 for syscall and break (passup from BIOS), 1 for program traps
	# (all remaining exceptions)
	# total: 4 vectors; 
	# each contains two areas: old and new; fill both PCs at start

	# every vector area contains:  
	# 1 for EntryHI, 1 for CAUSE, 
	# 1 for STATUS, and 1 for EPC (old area) or PC (new area)
	# 29 words for GPR ($0, k0 & k1 are 
	# excluded), 2 for HI & LO regs
	# total: 35 words of memory per area
	
	li 	$4, BIOSVECTPAGE
	li 	$5, BIOSPANIC # put here panic() address
	li 	$6, VECTAREANUM
Lfor:	
	sw	$5, VECTPCPOS($4)
	addiu	$4,VECTSIZE
	addi	$6, -1
	bgtz	$6, Lfor
	
	# finally, we set EntryHI and STATUS for kernel:
	# being EntryHI = 0, and STATUS good as it is now,
	# do not touch anything
	
	# and now let's start kernel: its starting address is 
	# in aout place for it
	li	$4, PROGVSTART
	lw	$31, 0($4)

	.set noreorder
	.set nomacro
	jr	$31
	rfe
	.set reorder
	.set macro	
	
	.end tapeboot

LtapebootEnd:
	.size	 tapeboot, LtapebootEnd - tapeboot
