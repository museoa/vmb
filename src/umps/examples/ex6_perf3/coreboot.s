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
 
oop:
  add     $2,$0,1
  j a7
a2: 
  sub     $2,$0,2
  mul     $3,$2,$2
  j j
a3:
  sub     $4,$3,$2
  div     $4,$3,2
  j o
a5:
  add     $2,$0,1
  sub     $2,$0,2
  j m
a4: 
  mul     $3,$2,$2
  sub     $4,$3,$2
  j oop
a6:  
  div     $4,$3,2
a7:  
  add     $2,$0,1
  j e
a9: 
  add     $2,$0,1
  j l
a: 
  sub     $2,$0,2
  mul     $3,$2,$2
  j f
b:
  sub     $4,$3,$2
  div     $4,$3,2
  j a9
c:
  add     $2,$0,1
  sub     $2,$0,2
  j b
d: 
  mul     $3,$2,$2
  sub     $4,$3,$2
  j f
e:  
  div     $4,$3,2
f:  
  add     $2,$0,1
g: 
  add     $2,$0,1
h: 
  sub     $2,$0,2
  mul     $3,$2,$2
i:
  sub     $4,$3,$2
  div     $4,$3,2
j:
  add     $2,$0,1
  sub     $2,$0,2
k: 
  mul     $3,$2,$2
  sub     $4,$3,$2
l:  
  div     $4,$3,2
m:  
  add     $2,$0,1
o: 
  add     $2,$0,1
p: 
  sub     $2,$0,2
  mul     $3,$2,$2
q:
  sub     $4,$3,$2
  div     $4,$3,2
r:
  add     $2,$0,1
  sub     $2,$0,2
s: 
  mul     $3,$2,$2
  sub     $4,$3,$2
t:  
  div     $4,$3,2
u:  
  add     $2,$0,1
v: 
  add     $2,$0,1
w: 
  sub     $2,$0,2
  mul     $3,$2,$2
x:
  sub     $4,$3,$2
  div     $4,$3,2
y:
  add     $2,$0,1
  sub     $2,$0,2
z: 
  mul     $3,$2,$2
  sub     $4,$3,$2

  j oop
  .space 64 
  .end main
LMainEnd:
  .size main, LMainEnd - main
  
  
