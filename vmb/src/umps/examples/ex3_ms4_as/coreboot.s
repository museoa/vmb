  .abicalls

  # Constants
  STATUSMASK       = 0xFDBFFFF3 # needed for calls to BIOS
  STACKFRAMESIZE   = 200          # no stack for now
  CLEARSCREEN      = 0xFFFFBFFF
  CLEARKB          = 0xFFFF7FFF 
  EXTINTNEGFLAG    = 0xFFFFFFFE  # kills the statusflag
  EXTINTENABLEFLAG = 0x00000001
  KEYBCHECK        = 0x00008000  # checks the interrupt 7
  SCREENCHK        = 0x00004000  # checks the interrupt 6
  RAM              = 0x20000004  # ram
  STATUSFLAG       = 0x20000000  # status for screeen
  TEMPSTACK        = 0x30000000  # use temporary stack
  KEYBADDR         = 0x1000000C  # address where keyboard is located
  SCREENADDR       = 0x1000025C  # address where screen is located
  .text                        # enter text segment
  
  # setup main function
  .globl main
  .type  main,@function
  .ent  main
  
main: 
  
   li $sp,TEMPSTACK
   li $27,RAM           #top of print stack
   li $26,RAM           #bottom of print stack
   sw $0,STATUSFLAG 
   
   addi $27,$27,4
   
   subu   $sp,STACKFRAMESIZE      # reserve stack frame
  .frame  $sp,STACKFRAMESIZE,$31  # set up frame, $31 is returnvalue register
  .mask   0x00000000,0            # we're not saving any register
  .fmask  0x00000000,0            # no are we savin any float register
  
 
  mfc0    $4, $12              # get cp0 Statusregister
  ori     $4, $4,0x0000FF01    # enable interrupts! 
  mtc0    $4, $12              # store cp0 Statusregister
  
foo:
   addi $8,$8,1
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   srl $8,1 
   j foo

bar:
 

  # prepare syscall
  mfc0    $4, $12              # get cp0 Statusregister
  li      $5, STATUSMASK       # disable bits for syscall, kernel mode, interrupts    
  and     $4, $4, $5           # apply bitmask of $5
  mtc0    $4, $12              # store cp0 Statusregister
  
  .set noreorder               # prevent assembler from reordering following instructions
  .set nomacro                # disables macros for following instructions

  li      $4, 3                # tell the bios that the HALT routine should be called
  nop  
  break                        # do break/syscall
  nop
  
  .set reorder                # reactivate assembler reordering
  .set macro                  # put macros back on

mainend:
  
  .space 0x180 + main - mainend
  
  mfc0 $4,$14
  sw   $4,0($sp)
  
  addi $sp,$sp,+4
  
  mfc0 $4,$13
  # check which interrupt it is
  li   $5,KEYBCHECK
  and  $5,$4,$5
  bgtz $5,keyboard
  li   $5,SCREENCHK
  and  $5,$4,$5
  bgtz $5,screen
  j finish

keyboard:
  mfc0 $4,$13
  and $4,CLEARKB
  mtc0 $4,$13
  
  lw   $7,KEYBADDR
  sw   $7,0($27)
  
  addi $27,$27,4
  
  addi $7,$27,-400
  bltz $7,skbraise
  
  li $27,RAM
  beq $27,$26,screen

skbraise: 
 
  lw   $7,STATUSFLAG
  beq  $0,$7,screen
  j finish

screen: 
  mfc0 $4,$13
  and $4,CLEARSCREEN
  mtc0 $4,$13
  

  lw   $7,0($26)
  andi $7,0x000000FF
  beq  $7,$0,screenclr
  
  sw   $7,SCREENADDR
  sw   $0,0($26)
  li   $7,1
  sw   $7,STATUSFLAG
  
  addi $26,$26,4
  addi $7,$26,-400
  bltz $7,finish
  li $26,RAM
  j finish
 
screenclr:
  li $27,RAM
  li $26,RAM
  sw $0,STATUSFLAG
  j finish
  
finish:

  addi $sp,$sp,-4
  lw   $6,0($sp)

  .set noreorder
  .set nomacro
  jr $6
  rfe
  .set reorder
  .set macro
  
  .end main
LMainEnd:
  .size main, LMainEnd - main
  
  