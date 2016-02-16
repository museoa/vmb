  .abicalls
   
  # Constants
  STATUSMASK     = 0xFDBFFFF3 # needed for calls to BIOS
  STACKFRAMESIZE = 0          # no stack for now
  TESTADRESS     = 0x20000020 # testaddress2

  .text                        # enter text segment
  
  # setup main function
  .globl main
  .type  main,@function
  .ent  main
  
main: 
  
   subu   $sp,STACKFRAMESIZE      # reserve stack frame
  .frame  $sp,STACKFRAMESIZE,$31  # set up frame, $31 is returnvalue register
  .mask  0x00000000,0            # we're not saving any register
  .fmask  0x00000000,0            # no are we savin any float register

  li $4,0x10000008
  lw $9,0($4)
  lw $11,4($4)
  andi $10,$11,0x00FF
  nop
  nop
  nop
  nop
  nop
  nop
  li $8,1
  nop
  nop
  
  # prepare syscall
  mfc0    $4, $12              # get cp0 Statusregister
  li      $5, STATUSMASK       # disable bits for syscall, kernel mode, interrupts    
  and     $4, $4, $5          # apply bitmask of $5
  mtc0    $4, $12              # store cp0 Statusregister

  .set noreorder               # prevent assembler from reordering following instructions
  .set nomacro                # disables macros for following instructions

  li      $4, 3                # tell the bios that the HALT routine should be called
  nop  
  break                        # do break/syscall
  nop
  
  .set reorder                # reactivate assembler reordering
  .set macro                  # put macros back on
  .end main
LMainEnd:
  .size main, LMainEnd - main
