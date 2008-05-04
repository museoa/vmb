 #$Id: crtso.s,v 1.1 2008-05-04 15:46:59 mbbh Exp $
	.abicalls

 # GCC automatic generated header
 
 # GNU C 2.7.2 [AL 1.1, MM 40] Linux/MIPSEL compiled by GNU C

 # Cc1 defaults:
 # -mgas -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -version -o
 # 

 	# useful constants

	PROGVSTART	= 0x20001004
	GPSTARTADDR	= 0x200010A8
	RAMBPHYADDR	= 0x20000000
	RAMSIZEADDR	= 0x10000004
	HALTBCODE 	= 3
	
	# This is the standard __start function for kernel
	# activation: it loads $t9 with .aout program start address,
	# $gp with base value for $gp computations from .text section, and 
	# at main() function return 
	# calls BIOS Halt() routine (a break with $4 == HALTBCODE)

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

	# computes stack bottom (last address of physical RAM) 
	# using $26 and $27 (k0 and k1)
	#lw 	$26, RAMBPHYADDR
	# lw 	$27, RAMSIZEADDR
	
	# add $26, $26, $27 
	# subu $26, $26, 4

	# loads start parameters into registers
	lw	$25, PROGVSTART
	li	$sp, 0x30000000
	lw	$gp, GPSTARTADDR

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

	# calls HALT BIOS routine directly
	li 	$4, HALTBCODE
	nop	
	break
	nop
	.set reorder
	.set macro	
	.end __start

L__startEnd:
	.size	__start, L__startEnd - __start



