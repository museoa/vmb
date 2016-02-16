 #$Id: crti.template.s,v 1.1 2008-05-04 15:46:59 mbbh Exp $
	.abicalls

 # GCC automatic generated header
 
 # GNU C 2.7.2 [AL 1.1, MM 40] Linux/MIPSEL compiled by GNU C

 # Cc1 defaults:
 # -mgas -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -version -o
 # 

	#this constant has to be set to a kernel-specific value
	TERMINATESYS = 0

	# remember that $sp has to be set by kernel too
	
 	# useful constants
	PROGVSTART	= 0x80000004
	GPSTARTADDR	= 0x800000A8
	
	# This is the standard __start function for generic program 
	# activation: it loads $t9 with starting address,
	# $gp with base value for $gp computations, and 
	# at main() function return calls kernel TERMINATE service  
	# (a SYSCALL with $2 == TERMINATE)

	.text
	.align	2
	.globl	__start
	.type	__start,@function
	.ent	__start
	.extern main 

__start:
	.frame	$fp,16,$31		# vars= 0, regs= 2/0, args= 0, extra= 8
	.mask	0x50000000,-4
	.fmask	0x00000000,0

	# $sp has to be set to correct value by kernel 
	li 	$24, PROGVSTART
	lw	$25, 0($24)
	li  $24, GPSTARTADDR
	lw	$gp, 0($24)

	subu	$sp,$sp,16
	.cprestore 0
	sw	$fp,12($sp)
	sw	$28,8($sp)
	move	$fp,$sp

	jal	main

	move	$sp,$fp	
	lw	$fp,12($sp)
	addu	$sp,$sp,16

	.set noreorder
	.set nomacro

	# calls kernel TERMINATE service
	li	$2, TERMINATESYS 
	nop	
	syscall
	nop
	.set reorder
	.set macro	
	.end __start

L__startEnd:
	.size	__start, L__startEnd - __start



