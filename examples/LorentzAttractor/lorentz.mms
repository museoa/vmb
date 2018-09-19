//
// Lorentz Attractor
//
// Equation:
//  dx/dt = sigma * (y - x)
//  dy/dt = rho * x - y - x * z
//  dz/dt = x * y - beta * z
//
// Parameters:
//  sigma = 10.0
//  rho   = 28.0
//  beta  = 8/3
//
// Initial conditions:
//  x_0 = 10.0
//  y_0 = 20.0
//  z_0 = 28.0
//
// Runge-Kutta 4th order:
//
//  k_0 = f(y)
//  k_1 = f(y + h/2 * k_0)
//  k_2 = f(y + h/2 * k_1)
//  k_3 = f(y + h * k_2)
//  y_h = y + h/6*(k_0 + 2 * k_1 + 2 * k_2 + k_3)
//
//  h = 0.003
//
// Plotting (x,y) for N=100000 steps in
//  x [-22,22]
//  y [-30,30]
// with nicely changing colors (not meaningful, mind you)
//
XSize		GREG	640
YSize		GREG	480
CPack		GREG	#100401
		LOC	#5000000000000000
VBase		GREG	@

		LOC	Data_Segment
N		OCTA	2000000
x0		OCTA	0.0			// 0.0
y0		OCTA	20.0			// 20.0
z0		OCTA	29.0			// 29.0
sigma		OCTA	10.0			// 10.0
rho		OCTA	28.0			// 28.0
beta		OCTA	2.666666666666667	// 8/3
h		OCTA	0.002			// 0.002
h2		OCTA	0.001			// h/2
h6		OCTA	3.333333333333333e-4		// h/6
Xshift		OCTA	-22.0			// -22.0
Xscale		OCTA	14.54545454545455	// 640.0/44.0
Yshift		OCTA	-30.0			// -30.0
Yscale		OCTA	8.0			// 480.0/60.0
CMask		OCTA	#ff00ff00ff00
Color0		OCTA	#0000000000ff
Colors		OCTA	#000000ff0000
		OCTA	#000000000001
		OCTA	#00ff00000000
		OCTA	#000000010000
		OCTA	#0000000000ff
		OCTA	#000100000000
Base		GREG	N


		LOC	#100
		
i		IS	$0
k		IS	$1
c		IS	$2
cval		IS	$3
cstep		IS	$4
x		IS	$5
y		IS	$6
z		IS	$7
args		IS	$8

Main		LDO	i,N
		XOR	k,k,k
		XOR	c,c,c
		LDO	cval,Color0
		LDO	args,CMask
		OR	cval,cval,args
		LDO	cstep,Colors
		LDO	x,x0
		LDO	y,y0
		LDO	z,z0
		// start main loop
		// compute next point
1H		OR	args+2,x,x
		OR	args+3,y,y
		OR	args+4,z,z
		PUSHJ	args+1,:RK4
		// store point
		OR	x,args+1,args+1
		OR	y,args+2,args+2
		OR	z,args+3,args+3
		// paint the point
		OR	args+3,cval,cval
		PUSHJ	args,:Paint
		// advance color
		CMP	args,k,#ff
		PBN	args,2F
		XOR	k,k,k
		LDO	args,CMask
		OR	cval,cval,args
		CMP	args,c,5
		PBN	args,3F
		SUB	c,c,6
3H		ADD	c,c,1
		LDA	args,Colors
		8ADDU	args,c,args
		LDO	cstep,args,0
2H		ADD	k,k,1
		WDIF	cval,cval,cstep
		// ready for the next step
		SUB	i,i,1
		PBNZ	i,1B
		// well done.
		TRAP	0,Halt,0

		PREFIX	:Paint
arg_x		IS	$0
arg_y		IS	$1
arg_cx		IS	$2
local		IS	$3

:Paint		LDO	local+0,:Xshift
		LDO	local+1,:Xscale
		FSUB	local+0,arg_x,local+0
		FMUL	local+0,local+0,local+1
		FIX	local+0,local+0
		BN	local+0,1F
		CMP	local+1,local+0,:XSize
		BNN	local+1,1F
		LDO	local+1,:Yshift
		LDO	local+2,:Yscale
		FSUB	local+1,arg_y,local+1
		FMUL	local+1,local+1,local+2
		FIX	local+1,local+1
		SUB	local+1,:YSize,local+1
		BN	local+1,1F
		CMP	local+2,local+1,:YSize
		BNN	local+2,1F
		MUL	local+1,local+1,:XSize
		ADD	local+0,local+0,local+1
		SL	local+0,local+0,2
		MOR	arg_cx,arg_cx,:CPack


//              we use either direct acces at VBase
//                STT     arg_cx,:VBase,local+0
//              SET     $255,0
//		TRAP    0,#10,0    // to force a sync 5

//              or we use a TRAP instead of direct access at VBase
		SL	$255,local+0,32		Hi Tetra is Address
		OR	$255,$255,arg_cx	LoTetra is RGB
		TRAP	0,#10,0			The GPutPixel TRAP

1H		POP	0,0


// Runge-Kutta
		PREFIX	:RK4
arg_x		IS	$0
arg_y		IS	$1
arg_z		IS	$2
ra		IS	$3
h		IS	$4
h2		IS	$5
h6		IS	$6

k0_x		IS	$7
k0_y		IS	$8
k0_z		IS	$9

k1_x		IS	$10
k1_y		IS	$11
k1_z		IS	$12

k2_x		IS	$13
k2_y		IS	$14
k2_z		IS	$15

k3_x		IS	$16
k3_y		IS	$17
k3_z		IS	$18

res_x		IS	$2
res_y		IS	$0
res_z		IS	$1

:RK4		GET	ra,:rJ
		LDO	h,:h
		LDO	h2,:h2
		LDO	h6,:h6
		OR	k0_x+1,arg_x,arg_x
		OR	k0_y+1,arg_y,arg_y
		OR	k0_z+1,arg_z,arg_z
		PUSHJ	k0_x,:Lorentz
		FMUL	k1_x+1,k0_x,h2
		FMUL	k1_y+1,k0_y,h2
		FMUL	k1_z+1,k0_z,h2
		FADD	k1_x+1,k1_x+1,arg_x
		FADD	k1_y+1,k1_y+1,arg_y
		FADD	k1_z+1,k1_z+1,arg_z
		PUSHJ	k1_x,:Lorentz
		FMUL	k2_x+1,k1_x,h2
		FMUL	k2_y+1,k1_y,h2
		FMUL	k2_z+1,k1_z,h2
		FADD	k2_x+1,k2_x+1,arg_x
		FADD	k2_y+1,k2_y+1,arg_y
		FADD	k2_z+1,k2_z+1,arg_z
		PUSHJ	k2_x,:Lorentz
		FMUL	k3_x+1,k2_x,h
		FMUL	k3_y+1,k2_y,h
		FMUL	k3_z+1,k2_z,h
		FADD	k3_x+1,k3_x+1,arg_x
		FADD	k3_y+1,k3_y+1,arg_y
		FADD	k3_z+1,k3_z+1,arg_z
		PUSHJ	k3_x,:Lorentz
		FADD	k0_x,k0_x,k3_x
		FADD	k0_y,k0_y,k3_y
		FADD	k0_z,k0_z,k3_z
		FADD	k1_x,k1_x,k2_x
		FADD	k1_y,k1_y,k2_y
		FADD	k1_z,k1_z,k2_z
		FADD	k1_x,k1_x,k1_x
		FADD	k1_y,k1_y,k1_y
		FADD	k1_z,k1_z,k1_z
		FADD	k0_x,k0_x,k1_x
		FADD	k0_y,k0_y,k1_y
		FADD	k0_z,k0_z,k1_z
		FMUL	k0_x,k0_x,h6
		FMUL	k0_y,k0_y,h6
		FMUL	k0_z,k0_z,h6
		PUT	:rJ,ra
		OR	ra,arg_x,arg_x
		FADD	res_y,arg_y,k0_y
		FADD	res_z,arg_z,k0_z
		FADD	res_x,ra,k0_x
		POP	3,0

		PREFIX	:Lorentz
x		IS	$0
y		IS	$1
z		IS	$2
sigma		IS	$3
rho		IS	$4
beta		IS	$5
y_x		IS	$6
rhoX		IS	$7
xz		IS	$8
betaZ		IS	$9
xy		IS	$10

res_x		IS	$2
res_y		IS	$0
res_z		IS	$1
:Lorentz	LDO	sigma,:sigma
		LDO	rho,:rho
		LDO	beta,:beta
		FSUB	y_x,y,x
		FMUL	rhoX,rho,x
		FMUL	xz,x,z
		FMUL	xy,x,y
		FMUL	betaZ,beta,z
		FSUB	rhoX,rhoX,y
		FMUL	res_x,sigma,y_x
		FSUB	res_y,rhoX,xz
		FSUB	res_z,xy,betaZ
		POP	3,0
