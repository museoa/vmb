		LOC	Data_Segment
		GREG	@
height	IS	48
width	IS	64
CellColor TETRA	#FF0000

Create	OCTA	0	three lists of records
Kill	OCTA	0
Unused	OCTA	0


x		IS	0	records contain 4 fields 
y		IS	4	in two octas
next	IS	8
esize	IS	16

Buffer	BYTE	0
BSize	IS	80
		LOC	Buffer+BSize
InArgs	OCTA	Buffer,BSize

Counts	BYTE	0	Byte-Array of Neighbor Counts
	LOC	Counts+height*width
	GREG	@	
Cells	BYTE	0	Byte-Array of cells 0 dead 1 alive
	LOC	Cells+height*width

	OCTA	0	ensure alignment of heap
Free	GREG	@	End of used data, begin of heap
	
	LOC 	#100

	PREFIX	:malloc		Memory allocate
size	IS	$0		parameter

:malloc ADDU	size,size,7	rund up to multiple of 8
	ANDN	size,size,7
	ADDU	:Free,:Free,size
	SUBU	$0,:Free,size
	POP	1,0

//	Painting Cells

	PREFIX	:paint paint a single pixel 


GPutPixel IS	#10	Trap number	
Gwidth	  IS	64	Physical length of a screen line

x	IS	$0
y	IS	$1
color	IS	$2
tmp	IS	$3

:paint	SET	tmp,Gwidth
	MUL	y,y,tmp
	ADD	y,y,x
	SL	y,y,32+2	times 4 and shift to hi tetra
	OR	$255,y,color	pass parameter to TRAP
	TRAP	0,GPutPixel,0
	POP	0,0

//	Basic list processing functions

	PREFIX	:remove		remove first element
list	IS	$0
eptr	IS	$1
nptr	IS	$2	

:remove	LDO	eptr,list,0
	BZ	eptr,empty
	LDO	nptr,eptr,:next
	STO	nptr,list,0
empty	SET	$0,eptr
	POP	1,0

	PREFIX	:add		add element as first element
list	IS	$0
eptr	IS	$1
nptr	IS	$2
:add	LDO	nptr,list,0
	STO	nptr,eptr,:next
	STO	eptr,list,0
	POP	0,0	

	PREFIX	:iterate	iterate over a list
first	IS	$0
proc	IS	$1		call this for each element
return	IS	$2
tmp	IS	$3

:iterate GET	return,:rJ
	JMP	2F

1H	SET	tmp+1,first
	PUSHGO	tmp,proc,0	
	LDO	first,first,:next
2H	BNZ	first,1B

	PUT	:rJ,return
	POP	0,0


	PREFIX	:dispose	return all elements of a list to unused
first	IS	$0
proc	IS	$1		call this for each element
return	IS	$2
tmp	IS	$3

:dispose GET	return,:rJ
	JMP	2F

1H	SET	tmp+1,first
	LDO	first,first,:next
	PUSHJ	tmp,:delete
2H	BNZ	first,1B

	PUT	:rJ,return
	POP	0,0


//	creating and deleting list elements

	PREFIX	:new		create new list element
x	IS	$0
y	IS	$1
return	IS	$2
eptr	IS	$3

:new	GET	return,:rJ
	LDA	eptr+1,:Unused
	PUSHJ	eptr,:remove
	BNZ	eptr,1F
	SET	eptr+1,:esize	
	PUSHJ	eptr,:malloc
1H	PUT	:rJ,return
	STT	x,eptr,:x
	STT	y,eptr,:y
	SET	$0,eptr
	POP	1,0

	PREFIX	:delete		return a list element to unused
eptr	IS	$0
return	IS	$1
tmp	IS 	$3
:delete	GET	return,:rJ
	LDA	tmp+1,:Unused
	SET	tmp+2,eptr
	PUSHJ	tmp,:add
	PUT	:rJ,return
	POP	1,0


//	Second level functions called in Main

	PREFIX	:Init	Initialize the Create List

glider	BYTE	5,1,0,2,1,0,2,1,2,2,2 		n,dx1,dy1,...,dxn,dyn
	TETRA	0				alignment

pulse	BYTE	22,0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0
	BYTE	   0,1,2,1,3,1,4,1,5,1,7,1
	BYTE	   0,2,1,2,2,2,3,2,4,2,5,2,6,2,7,2

x	IS	$0
y	IS	$1
dx	IS	$2
dy	IS	$3
n	IS	$4
p	IS	$5
return	IS	$6
tmp	IS	$7

:Init	GET	return,:rJ

	GETA	tmp+1,glider
	SET	tmp+2,4
	SET	tmp+3,4
	PUSHJ	tmp,:pattern

	GETA	tmp+1,pulse
	SET	tmp+2,20
	SET	tmp+3,20
	PUSHJ	tmp,:pattern

	PUT	:rJ,return
	POP	0,0

	PREFIX :pattern
p	IS	$0
x	IS	$1
y	IS	$2
dx	IS	$3
dy	IS	$4
n	IS	$5
return	IS	$6
tmp	IS	$7

:pattern GET	return,:rJ
	LDB	n,p,0
	ADDU	p,p,1
loop	BZ	n,end
	LDB	dx,p,0
	LDB	dy,p,1

	ADD	tmp+1,x,dx
	ADD	tmp+2,y,dy
	PUSHJ	tmp,:docreate

	ADDU	p,p,2
	SUB	n,n,1
	JMP	loop

end	PUT	:rJ,return
	POP	0,0




	PREFIX	:Update		Update the count array

return	IS	$0
tmp	IS	$1

:Update GET	return,:rJ

	LDO	tmp+1,:Create
	GETA	tmp+2,:birth
	PUSHJ	tmp,:iterate

	LDO	tmp+1,:Kill
	GETA	tmp+2,:death
	PUSHJ	tmp,:iterate

	PUT	:rJ,return
	POP	0,0
	
	PREFIX	:birth

eptr	IS	$0
return	IS	$1
tmp	IS	$5

:birth	GET	return,:rJ
	LDT	tmp+1,eptr,:x
	LDT	tmp+2,eptr,:y
	SET	tmp+3,1
	PUSHJ	tmp,:adjust
end	PUT	:rJ,return
	POP	0,0

	PREFIX	:death

eptr	IS	$0
return	IS	$1
tmp	IS	$5

:death	GET	return,:rJ
	LDT	tmp+1,eptr,:x
	LDT	tmp+2,eptr,:y
	ORN	tmp+3,tmp+3,tmp+3	minus one
	PUSHJ	tmp,:adjust
end	PUT	:rJ,return
	POP	0,0



	PREFIX	:adjust adjust neighbour counts of x/y by diff

x	IS	$0
y	IS	$1
diff	IS	$2
p	IS	$3
tmp	IS	$4

:adjust	LDA	p,:Counts
	ADDU	p,p,x
	MUL	tmp,y,:width
	ADDU	p,p,tmp		points to center element

	SUB	p,p,:width+1	top-left	
	LDB	tmp,p,0
	ADD	tmp,tmp,diff
	STB	tmp,p,0

	LDB	tmp,p,1		top-mid
	ADD	tmp,tmp,diff
	STB	tmp,p,1

	LDB	tmp,p,2		top-right
	ADD	tmp,tmp,diff
	STB	tmp,p,2

	ADD	p,p,:width	mid-left
	LDB	tmp,p,0
	ADD	tmp,tmp,diff
	STB	tmp,p,0

	LDB	tmp,p,2		mid-right
	ADD	tmp,tmp,diff
	STB	tmp,p,2

	ADD	p,p,:width	bot-left
	LDB	tmp,p,0
	ADD	tmp,tmp,diff
	STB	tmp,p,0

	LDB	tmp,p,1		bot-mid
	ADD	tmp,tmp,diff
	STB	tmp,p,1

	LDB	tmp,p,2		bot-right
	ADD	tmp,tmp,diff
	STB	tmp,p,2

	POP	0,0

	PREFIX	:NextGeneration
//	iterate over neighbours of created or killed cells and
//	make new lists of cells to create or kill, 
//	set the cell array and the screen

created	IS	$0
killed	IS	$1
return	IS	$2
tmp	IS	$3

:NextGeneration  GET	return,:rJ
	LDO	created,:Create
	LDO	killed,:Kill
	SET	tmp,0
	STO	tmp,:Create   make lists empty
	STO	tmp,:Kill	

	SET	tmp+1,created
	GETA	tmp+2,:test
	PUSHJ	tmp,:iterate

	SET	tmp+1,killed
	GETA	tmp+2,:test
	PUSHJ	tmp,:iterate

	SET	tmp+1,created
	PUSHJ	tmp,:dispose
	SET	tmp+1,killed
	PUSHJ	tmp,:dispose

	PUT	:rJ,return
	POP	0,0

	PREFIX	:test all neighbours of the element

eptr	IS	$0
return	IS	$1
x	IS	$2
y	IS	$3
tmp	IS	$4

:test	GET	return,:rJ
	LDT	x,eptr,:x
	LDT	y,eptr,:y

	SUB	tmp+1,x,1	top-left
	SUB	tmp+2,y,1
	PUSHJ	tmp,:decide
	SET	tmp+1,x		top-mid
	SUB	tmp+2,y,1
	PUSHJ	tmp,:decide
	ADD	tmp+1,x,1	top-right
	SUB	tmp+2,y,1
	PUSHJ	tmp,:decide

	SUB	tmp+1,x,1	mid-left
	SET	tmp+2,y
	PUSHJ	tmp,:decide
	SET	tmp+1,x		mid-mid
	SET	tmp+2,y
	PUSHJ	tmp,:decide
	ADD	tmp+1,x,1	mid-right
	SET	tmp+2,y
	PUSHJ	tmp,:decide

	SUB	tmp+1,x,1	bot-left
	ADD	tmp+2,y,1
	PUSHJ	tmp,:decide
	SET	tmp+1,x		bot-mid
	ADD	tmp+2,y,1
	PUSHJ	tmp,:decide
	ADD	tmp+1,x,1	bot-right
	ADD	tmp+2,y,1
	PUSHJ	tmp,:decide

	PUT	:rJ,return
	POP	0,0


	PREFIX	:decide what to do with cell x,y kill, create, or survive
x	IS	$0
y	IS	$1
index	IS	$2
count	IS	$3
state	IS	$4
base	IS	$5
return 	IS	$6
tmp	IS	$7

:decide BN	x,quit		check for bounds
	BN	y,quit
	CMP	tmp,x,:width
	BNN	tmp,quit
	CMP	tmp,y,:height
	BNN	tmp,quit
	
	GET	return,:rJ
	MUL	tmp,y,:width
	ADD	index,tmp,x
	LDA	base,:Cells
	LDB	state,base,index
	LDA	base,:Counts
	LDB	count,base,index
	BZ	state,dead
	
	CMP	tmp,count,2
	BZ	tmp,end
	CMP	tmp,count,3
	BZ	tmp,end

	SET	tmp+1,x
	SET	tmp+2,y		
	PUSHJ	tmp,:dokill
	
	JMP	end

dead	CMP	tmp,count,3
	BNZ	tmp,end

	SET	tmp+1,x
	SET	tmp+2,y		
	PUSHJ	tmp,:docreate
	
end	PUT	:rJ,return
quit	POP	0,0

	PREFIX :docreate create the cell, update cells, screen, add Created
x	IS	$0
y	IS	$1
return	IS	$2
base	IS	$3
tmp	IS	$4


:docreate GET	return,:rJ
	LDA	base,:Cells
	MUL	tmp,y,:width
	ADD	tmp,tmp,x
	ADD	base,base,tmp
	LDB	tmp,base,0
	BP	tmp,end		already alive
	SET	tmp,1
	STB	tmp,base,0

	SET	tmp+1,x
	SET	tmp+2,y
	LDTU	tmp+3,:CellColor
	PUSHJ	tmp,:paint
	
	LDA	tmp+1,:Create

	SET	tmp+3,x
	SET	tmp+4,y		
	PUSHJ	tmp+2,:new
	
	PUSHJ	tmp,:add

end	PUT	:rJ,return
	POP	0,0



	PREFIX :dokill kill the cell, update cells and screen, add to kill
x	IS	$0
y	IS	$1
return	IS	$2
base	IS	$3
tmp	IS	$4


:dokill GET	return,:rJ
	LDA	base,:Cells
	MUL	tmp,y,:width
	ADD	tmp,tmp,x
	ADD	base,base,tmp
	LDB	tmp,base,0
	BZ	tmp,end		already dead
	SET	tmp,0
	STB	tmp,base,0

	SET	tmp+1,x
	SET	tmp+2,y
	SET	tmp+3,0
	PUSHJ	tmp,:paint
	
	LDA	tmp+1,:Kill

	SET	tmp+3,x
	SET	tmp+4,y		
	PUSHJ	tmp+2,:new
	
	PUSHJ	tmp,:add

end	PUT	:rJ,return
	POP	0,0




	PREFIX	:

TWait	IS	#11	Trap number	

Main	PUSHJ	$0,Init
1H	PUSHJ	$0,Update
	PUSHJ	$0,NextGeneration
	TRAP	0,TWait,0
	BNZ	$255,1B
	SET	$255,0
	TRAP	0,Halt,0
