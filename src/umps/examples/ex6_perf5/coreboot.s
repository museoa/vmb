  .abicalls
  
  .text                        # enter text segment
  
  # setup main function
  .globl main
  .type  main,@function
  .ent  main
  
main: 
  
  .frame  $sp,0,$31  # set up frame, $31 is returnvalue register
  .mask   0x00000000,0            # we're not saving any register
  .fmask  0x00000000,0            # no are we savin any float register
 
l:
  li $2,0x20001000
  sw $0,0($2)
  nop
  sw $0,4($2)
  nop
  sb $0,5($2)
  nop
  lb $3,5($2)
  nop
  sw $3,0($2)
  nop
  lbu $3,2($2)
  nop
  sw $0,100($2)
  nop
  j l
  .space 64 
  .end main
LMainEnd:
  .size main, LMainEnd - main
  
  
