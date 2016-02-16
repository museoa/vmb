 #$Id: coreboot.s,v 1.1 2008-05-04 15:46:59 mbbh Exp $

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

	PADDINGTLB	= 0x100 - 8
	PADDINGEXC	= 0x80 - 16

	#Code start

	.text
	.align	2
	.globl	coreboot
	.type	coreboot,@function
	.ent	coreboot

coreboot:
	.frame	$fp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set noat

	.set noreorder
	.set nomacro

	# boot starts here 
	j LCoreBootStart
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

LCoreBootStart:	
	# mapping all exception vectors to BIOS code
	# this is done resetting bit 22 in STATUS CP0 register
	# we also assure that kernel will start in kernel mode, with
	# interrupts and VM disabled: this is done zeroing bits 3, 2 and 25
	mfc0	$4, $12
	li	$5, STATUSMASK   
	and	$4, $4, $5
	mtc0	$4, $12
	
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
	
	.end coreboot

LcorebootEnd:
	.size	 coreboot, LcorebootEnd - coreboot
