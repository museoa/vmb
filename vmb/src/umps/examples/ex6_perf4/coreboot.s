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
  add     $2,$0,1
  sub     $2,$0,2
  mul     $3,$2,$2
  sub     $4,$3,$2
  div     $4,$3,2
  add     $2,$0,1
  li      $2,0x20001000
  sw      $4,0($2)
  sub     $2,$0,2
  mul     $3,$2,$2
  sub     $4,$3,$2
  div     $4,$3,2
  li      $2,0x20001000
  lw      $4,0($2)

  j l
  .space 64 
  .end main
LMainEnd:
  .size main, LMainEnd - main
  
  
