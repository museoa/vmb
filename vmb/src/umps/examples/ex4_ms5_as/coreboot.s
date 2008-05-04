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
  
 
foo:
   addi $8,$8,1
   srl $8,1 
   addi $9,$8,3
   mul  $9,$9,3
   sub  $8,$9,3
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

LMainEnd:
  .size main, LMainEnd - main
  
  
