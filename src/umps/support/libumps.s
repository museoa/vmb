#$Id: libumps.s,v 1.1 2008-05-04 15:47:00 mbbh Exp $
	
	.abicalls

 # GCC automatic generated header
 
 # GNU C 2.7.2 [AL 1.1, MM 40] Linux/MIPSEL compiled by GNU C

 # Cc1 defaults:
 # -mgas -mabicalls

 # Cc1 arguments (-G value = 0, Cpu = 3000, ISA = 1):
 # -quiet -dumpbase -version -o
 # 

 	# Constants
	
	# BIOS BREAK codes

	FORKBCODE	= 0
	LDSTBCODE	= 1
	PANICBCODE	= 2
	HALTBCODE	= 3
	
	# miscellaneous

	# ASIDMASK	= 0x00000FC0	

    KEYBBUFFSPTR     = 0x1E000000
    KEYBBUFFGPTR     = 0x1E000004
    KEYBBUFFSTART    = 0x1E000008
    KEYBBUFFEND      = 0x1E00003A
    KEYBDEVID        = 0x1000000C 
    
    SCREENBUFFSPTR   = 0x1E00003E
    SCREENBUFFGPTR   = 0x1E000042
    SCREENBUSYFLAG   = 0x1E000046
    SCREENBUFFSTART  = 0x1E000047
    SCREENBUFFEND    = 0x1E000079
    SCREENDEVID      = 0x1000025C


	# Code start

	.text
	.set noat
	.align 2


	# this function cause a system call trap
	# system call code is in $4 (a0) register
	# return value in $2 (v0) register
	# it is programmer's task to load the return value into 
 	# state register, and to set PC correctly for returning _after_
	# syscall

	.globl	SYSCALL
	.type	SYSCALL, @function
	.ent 	SYSCALL

SYSCALL:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	syscall
	nop
	.set reorder
	.set macro
	jr	$31
	
	.end SYSCALL

LendSYSCALL:
	.size	SYSCALL, LendSYSCALL - SYSCALL
		
# begin addition by Martin Hauser <info@martin-hauser.net>

    .globl getchr
    .type  getchr,@function
    .ent   getchr

getchr:
    .frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
    # load $4 <- KEYBBUFFGPTR
    li   $4,KEYBBUFFGPTR
    lw   $4,0($4)
    
    li   $5,KEYBBUFFSTART
    add  $5,$4,$5
    
    # check sanity of get pointer
    li   $6,KEYBBUFFEND
    sub  $6,$6,$5
	bltz $6,LFailedGetChr
	
	lbu  $2,0($5)
	bne  $6,$0,LGCSkip
	li   $4,-1

LGCSkip:
    addi $4,$4,1
    
    # store $4 -> KEYBBUFFGPTR
    li   $5,KEYBBUFFGPTR
    sw   $4,0($5)
    jr   $31
#END
	
LFailedGetChr:
    li   $2,-1
    jr   $31
#END

    .end getchr
#END

LgetchrEnd:
    .size getchr, LgetchrEnd - getchr
#END

# end addition by Martin Hauser <info@martin-hauser.net>


	# This function returns the CP0 INDEX register 
	# return value goes back  thru register $2 (v0)

	.globl	getINDEX
	.type	getINDEX, @function
	.ent 	getINDEX
	
getINDEX:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $0
	nop
	.set reorder
	.set macro
	jr	$31

	.end getINDEX

LendgetINDEX:
	.size	getINDEX, LendgetINDEX - getINDEX


	# This function returns the CP0 RANDOM register 
	# return value goes back  thru register $2 (v0)

	.globl	getRANDOM
	.type	getRANDOM, @function
	.ent 	getRANDOM
	
getRANDOM:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $1
	nop
	.set reorder
	.set macro
	jr	$31

	.end getRANDOM

LendgetRANDOM:
	.size	getRANDOM, LendgetRANDOM - getRANDOM



	# This function returns the CP0 ENTRYLO register 
	# return value goes back  thru register $2 (v0)

	.globl	getENTRYLO
	.type	getENTRYLO, @function
	.ent 	getENTRYLO
	
getENTRYLO:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $2
	nop
	.set reorder
	.set macro
	jr	$31

	.end getENTRYLO

LendgetENTRYLO:
	.size	getENTRYLO, LendgetENTRYLO - getENTRYLO



	# This function returns the CP0 BADVADDR register 
	# return value goes back  thru register $2 (v0)

	.globl	getBADVADDR
	.type	getBADVADDR, @function
	.ent 	getBADVADDR
	
getBADVADDR:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $8
	nop
	.set reorder
	.set macro
	jr	$31

	.end getBADVADDR

LendgetBADVADDR:
	.size	getBADVADDR, LendgetBADVADDR - getBADVADDR



	# This function returns the CP0 ENTRYHI register 
	# return value goes back  thru register $2 (v0)

	.globl	getENTRYHI
	.type	getENTRYHI, @function
	.ent 	getENTRYHI
	
getENTRYHI:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $10
	nop
	.set reorder
	.set macro
	jr	$31

	.end getENTRYHI

LendgetENTRYHI:
	.size	getENTRYHI, LendgetENTRYHI - getENTRYHI



	# This function returns the CP0 STATUS register 
	# return value goes back  thru register $2 (v0)

	.globl	getSTATUS
	.type	getSTATUS, @function
	.ent 	getSTATUS
	
getSTATUS:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $12
	nop
	.set reorder
	.set macro
	jr	$31

	.end getSTATUS

LendgetSTATUS:
	.size	getSTATUS, LendgetSTATUS - getSTATUS


	# This function returns the CP0 CAUSE register 
	# return value goes back  thru register $2 (v0)

	.globl	getCAUSE
	.type	getCAUSE, @function
	.ent 	getCAUSE
	
getCAUSE:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $13
	nop
	.set reorder
	.set macro
	jr	$31

	.end getCAUSE

LendgetCAUSE:
	.size	getCAUSE, LendgetCAUSE - getCAUSE



	# This function returns the CP0 EPC register 
	# return value goes back  thru register $2 (v0)

	.globl	getEPC
	.type	getEPC, @function
	.ent 	getEPC
	
getEPC:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $14
	nop
	.set reorder
	.set macro
	jr	$31

	.end getEPC

LendgetEPC:
	.size	getEPC, LendgetEPC - getEPC


	# This function returns the CP0 PRID register 
	# return value goes back  thru register $2 (v0)

	.globl	getPRID
	.type	getPRID, @function
	.ent 	getPRID
	
getPRID:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mfc0	$2, $15
	nop
	.set reorder
	.set macro
	jr	$31

	.end getPRID

LendgetPRID:
	.size	getPRID, LendgetPRID - getPRID


	# This function returns the CP0 INDEX register after write 
	# argument comes thru register $4 (a0)
	# return value goes back  thru register $2 (v0)

	.globl	setINDEX
	.type	setINDEX, @function
	.ent 	setINDEX
	
setINDEX:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mtc0	$4, $0
	nop
	mfc0	$2, $0
	nop
	.set reorder
	.set macro
	jr	$31

	.end setINDEX

LendsetINDEX:
	.size	setINDEX, LendsetINDEX - setINDEX


	# This function returns the CP0 ENTRYLO register after write
	# argument comes thru register $4 (a0)
	# return value goes back  thru register $2 (v0)

	.globl	setENTRYLO
	.type	setENTRYLO, @function
	.ent 	setENTRYLO
	
setENTRYLO:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mtc0	$4, $2
	nop
	mfc0	$2, $2
	nop
	.set reorder
	.set macro
	jr	$31

	.end setENTRYLO

LendsetENTRYLO:
	.size	setENTRYLO, LendsetENTRYLO - setENTRYLO



	# This function returns the CP0 ENTRYHI register after write
	# argument goes thru register $4 (a0)
	# return value goes back  thru register $2 (v0)

	.globl	setENTRYHI
	.type	setENTRYHI, @function
	.ent 	setENTRYHI
	
setENTRYHI:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mtc0	$4, $10
	nop
	mfc0	$2, $10
	nop
	.set reorder
	.set macro
	jr	$31

	.end setENTRYHI

LendsetENTRYHI:
	.size	setENTRYHI, LendsetENTRYHI - setENTRYHI



	# This function returns the CP0 STATUS register after write
	# argument goes thru register $4 (a0)
	# return value goes back  thru register $2 (v0)

	.globl	setSTATUS
	.type	setSTATUS, @function
	.ent 	setSTATUS
	
setSTATUS:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mtc0	$4, $12
	nop
	mfc0	$2, $12
	nop
	.set reorder
	.set macro
	jr	$31

	.end setSTATUS

LendsetSTATUS:
	.size	setSTATUS, LendsetSTATUS - setSTATUS


	# This function returns the CP0 CAUSE register after write
	# argument goes thru register $4 (a0)
	# return value goes back  thru register $2 (v0)

	.globl	setCAUSE
	.type	setCAUSE, @function
	.ent 	setCAUSE
	
setCAUSE:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	mtc0	$4, $13
	nop
	mfc0	$2, $13
	nop
	.set reorder
	.set macro
	jr	$31

	.end setCAUSE

LendsetCAUSE:
	.size	setCAUSE, LendsetCAUSE - setCAUSE


	# This function write in a random TLB position
	# actual ENTRYHI, ENTRYLO registers

	.globl	TLBWR
	.type	TLBWR, @function
	.ent 	TLBWR
	
TLBWR:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0

	
	.set noreorder
	.set nomacro
	nop
	tlbwr
	nop
	.set reorder
	.set macro

	jr	$31

	.end TLBWR

LendTLBWR:
	.size	TLBWR, LendTLBWR - TLBWR


	
	# This function write at INDEX TLB position 
  	# actual ENTRYHI and ENTRYLO

	.globl	TLBWI
	.type	TLBWI, @function
	.ent 	TLBWI
	
TLBWI:
	
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	tlbwi
	nop
	.set reorder
	.set macro

	jr	$31

	.end TLBWI

LendTLBWI:
	.size	TLBWI, LendTLBWI - TLBWI


	# This function probes TLB for matching 
	# arguments ENTRYHI, ENTRYLO

	.globl	TLBP
	.type	TLBP, @function
	.ent 	TLBP
	
TLBP:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	tlbp
	nop
	.set reorder
	.set macro

	jr	$31

	.end TLBP

LendTLBP:
	.size	TLBP, LendTLBP - TLBP



	# This function reads TLB entry at INDEX and returns in ENTRYHI and ENTRYLO

	.globl	TLBR
	.type	TLBR, @function
	.ent 	TLBR
	
TLBR:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	nop
	tlbr
	nop
	.set reorder
	.set macro
	jr	$31

	.end TLBR

LendTLBR:
	.size	TLBR, LendTLBR - TLBR


	# This function issues a TLBCLR instruction
 	# no arguments or return values
 
 	.globl	TLBCLR
 	.type	TLBCLR, @function
 	.ent 	TLBCLR
 	
TLBCLR:
 	.frame  $fp,0,$31
 	.mask   0x00000000,0
 	.fmask  0x00000000,0
 	
 	.set noreorder
 	.set nomacro
 	nop
 	mtc0	$4, $4
 	nop
 	.set reorder
 	.set macro
 	jr	$31
 
 	.end TLBCLR
 
LendTLBCLR:
 	.size	TLBCLR, LendTLBCLR - TLBCLR
 


	# This function loads a processor state from memory and start 
	# executing it. It changes processor state completely, and it is
	# NOT an atomic operation (see interface for description)

	.globl	FORK
	.type	FORK, @function
	.ent 	FORK
	
FORK:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0

	# first registers loaded are HI and LO
	lw	$1, 132($7)
	mthi	$1
	lw	$1, 136($7)
	mtlo	$1
	
	lw	$1, 16($7)
	
	# register $2 itself is not loaded
	
	lw	$3, 24($7)
	
	#registers $4..$7 are not loaded too

	lw	$8, 44($7)
	lw	$9, 48($7)
	lw	$10, 52($7)
	lw	$11, 56($7)
	lw	$12, 60($7)
	lw	$13, 64($7) 	
	lw	$14, 68($7)
	lw	$15, 72($7)
	lw	$16, 76($7)
	lw	$17, 80($7)
	lw	$18, 84($7)
	lw	$19, 88($7)
	lw	$20, 92($7)
	lw	$21, 96($7)
	lw	$22, 100($7)
	lw	$23, 104($7)
	lw	$24, 108($7)
	lw	$25, 112($7)

	# $26 and $27 are not saved so they are not loaded too

	lw	$28, 116($7)
	lw	$29, 120($7)
	lw	$30, 124($7)
	lw	$31, 128($7)
	# all processor registers loaded (almost)

	# load CAUSE from memory to $7 (a3)
	lw	$7, 4($7)

	# move $4 to $2 to use $4 as EXEC call parameter
	move 	$2, $4
	
	.set noreorder
	.set nomacro
	li	$4, FORKBCODE
	break
	nop
	.set reorder
	.set macro


	jr	$31
	
	.end FORK

LendFORK:
	.size	FORK, LendFORK - FORK


	# This function will save processor status to memory block pointed by
	# register $4 (a0), and return PC value of instruction immediately
	# following the call as return value in $2.
	# PC field itself is intentionally leaved at 0 value

	.globl	STST
	.type	STST, @function
	.ent 	STST
	
STST:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noat
	sw	$1, 16($4)
	sw	$2, 20($4)

	# gets CAUSE register and stores it
	mfc0	$2, $13
	sw 	$2, 4($4)

	sw	$3, 24($4)
	sw	$4, 28($4)
	sw	$5, 32($4)
	sw	$6, 36($4)
	sw	$7, 40($4)
	sw	$8, 44($4)
	sw	$9, 48($4)
	sw	$10, 52($4)
	sw	$11, 56($4)
	sw	$12, 60($4)
	sw	$13, 64($4) 	
	sw	$14, 68($4)
	sw	$15, 72($4)
	sw	$16, 76($4)
	sw	$17, 80($4)
	sw	$18, 84($4)
	sw	$19, 88($4)
	sw	$20, 92($4)
	sw	$21, 96($4)
	sw	$22, 100($4)
	sw	$23, 104($4)
	sw	$24, 108($4)
	sw	$25, 112($4)

	# $26 and $27 are not saved

	sw	$28, 116($4)
	sw	$29, 120($4)
	sw	$30, 124($4)
	sw	$31, 128($4)
	mfhi	$5
	sw	$5, 132($4)
	mflo	$5
	sw	$5, 136($4)
	# all processor registers saved
	
	# gets EntryHI and stores it 
	mfc0	$2, $10
	sw	$2, 0($4)
	

	# and now saves STATUS register and zeroes PC
	mfc0	$2, $12
	sw	$2, 8($4)
	sw	$0, 12($4)

	# reloads $3 and $5 registers

	lw	$3, 24($4)
	lw 	$5, 32($4)

	# sets $2 to return value

	move	$2, $31	

	jr	$31

	.end STST

LendSTST:
	.size	STST, LendSTST - STST


	.globl	HALT
	.type	HALT, @function
	.ent 	HALT
	
HALT:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	li	$4, HALTBCODE
	break
	nop
	.set reorder
	.set macro


	jr	$31

	.end HALT

LendHALT:
	.size	HALT, LendHALT - HALT


	.globl	PANIC
	.type	PANIC, @function
	.ent 	PANIC
	
PANIC:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	.set noreorder
	.set nomacro
	li	$4, PANICBCODE
	break
	nop
	.set reorder
	.set macro


	jr	$31

	.end PANIC

LendPANIC:
	.size	PANIC, LendPANIC - PANIC


	# This function force the complete reload of processor state from
	# vector state area pointed by argument in $4 (a0): it works only in 
	# kernel state. There is  no real return: $5 is used as BIOS 
	# argument, but it is reloaded too


	.globl	LDST
	.type	LDST, @function
	.ent 	LDST
	
LDST:
	.frame  $fp,0,$31
	.mask   0x00000000,0
	.fmask  0x00000000,0
	
	move	$5, $4
	
	.set noreorder
	.set nomacro
	li	$4, LDSTBCODE
	break
	nop
	.set reorder
	.set macro


	jr	$31

	.end LDST

LendLDST:
	.size	LDST, LendLDST - LDST
