	.abicalls
 	
  # Constants
	STATUSMASK 	   = 0xFDBFFFF3 # needed for calls to BIOS
	STACKFRAMESIZE = 0          # no stack for now
	TESTADRESS     = 0x20000020 # testaddress2

	.text												# enter text segment
	
	# setup main function
	.globl main
	.type	main,@function
	.ent	main
	
main: 
	
	 subu   $sp,STACKFRAMESIZE      # reserve stack frame
	.frame  $sp,STACKFRAMESIZE,$31  # set up frame, $31 is returnvalue register
	.mask	  0x00000000,0            # we're not saving any register
	.fmask  0x00000000,0            # no are we savin any float register

	## Part 1: arithmetics
	
	li   $4,8      # first argument is 8
	li   $5,2      # second argument is 2
	
	add  $6,$4,$5  # $6 = $4 + $5 (should be 10)
	sub  $7,$4,$5  # $7 = $4 - $5 (should be 6)
	
	mult $4,$5     # LO = $4 * $5 (should be 16)
	mflo $8				 # $8 = LO
	nop						 # needed cause of mips pipline
	nop						 #
	
	div  $4,$5     # LO = $4 / $5 (should be 4)
	mflo $9			   # $9 = LO
	nop
	nop
	
	## Part 2: Memory operations
	
	li $11,TESTADRESS # load address to store data to
	sw $4, 0($11)      # store $4 -> TESTADDRESS2
	lw $5, 0($11)      # $4 < TESTADDRESS2, $4 is equal to $5
	
	
	## Part 2: Bit Operations
	
	li $4,8 # load operands
	li $5,2 # load operands
	sll $4,$4,1 # $4 = 16 now
	ori $4,$4,1 # $4 = 17 now
	srl $4,$4,1 # $4 = 8 again
	
	## Part 4: Calling halt
	
	# prepare syscall
  mfc0    $4, $12							# get cp0 Statusregister
  li      $5, STATUSMASK     	# disable bits for syscall, kernel mode, interrupts    
  and     $4, $4, $5					# apply bitmask of $5
	mtc0    $4, $12							# store cp0 Statusregister

	.set noreorder 							# prevent assembler from reordering following instructions
	.set nomacro								# disables macros for following instructions

	li      $4, 3								# tell the bios that the HALT routine should be called
	nop	
	break												# do break/syscall
	nop
	
	.set reorder								# reactivate assembler reordering
	.set macro									# put macros back on
	.end main
LMainEnd:
	.size main, LMainEnd - main