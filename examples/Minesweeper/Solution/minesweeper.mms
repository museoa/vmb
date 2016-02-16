************************************************
* MINESWEEPER
* Autor: M.Ruckert
************************************************

* Constants

Rows		IS 9
Columns 	IS 13
Mines		IS 10


* Data

		LOC	Data_Segment

	
MineField	GREG	@      Boolean field 1 = Mine, 0 = empty
1H		BYTE	0
		LOC	1B+Rows*Columns
		
MineCounts	GREG	@     Number of mines in neighboring cells incl. self
1H		BYTE	0
		LOC	1B+Rows*Columns
		
FieldVisible	GREG	@     Boolean field 1 = visible, 0 = hidden
1H		BYTE	0
		LOC	1B+Rows*Columns
VisibleCount	GREG	0

* Input string
InBuffer	BYTE	0,0,0
InArg		OCTA	InBuffer
		OCTA	3



XORIGIN		IS	20
YORIGIN		IS	60
WIDTH		IS	21



* Basic Subroutines
		LOC	#100

		PREFIX	:IncMineCount:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
Offset		IS  $2
Temp		IS  $3


* increments the Mine Count at (x,y)
		
:IncMineCount	BN	x,Return	Test Range of Parameters
		BN	y,Return
		CMP	Temp,x,:Columns
		BNN	Temp,Return
		CMP	Temp,y,:Rows
		BNN	Temp,Return

		MUL	Offset,y,:Columns
		ADD	Offset,Offset,x
		LDB	Temp,:MineCounts,Offset
		ADD	Temp,Temp,1
		STB	Temp,:MineCounts,Offset
		
Return		POP	0,0


		PREFIX	:GetMine:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
Offset		IS  $2
Temp		IS  $3


* get the MineField at (x,y)
		
:GetMine	BN	x,ReturnZero	Test Range of Parameters
		BN	y,ReturnZero
		CMP	Temp,x,:Columns
		BNN	Temp,ReturnZero
		CMP	Temp,y,:Rows
		BNN	Temp,ReturnZero

		MUL	Offset,y,:Columns
		ADD	Offset,Offset,x
		LDB	$0,:MineField,Offset
		POP	1,0
		
ReturnZero	SET	$0,0
		POP	1,0

		PREFIX	:PutMine:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
Offset		IS  $2
i		IS  $3
k		IS  $4
rJ		IS  $5
Temp		IS  $6


* places a mine in the MineField at (x,y) 
* and updates MineCounts
* returns 1 if successfull, 0 else
		
:PutMine	BN	x,Fail	Test Range of Parameters
		BN	y,Fail
		CMP	Temp,x,:Columns
		BNN	Temp,Fail
		CMP	Temp,y,:Rows
		BNN	Temp,Fail

		MUL	Offset,y,:Columns
		ADD	Offset,Offset,x
		LDB	Temp,:MineField,Offset
		BNZ	Temp,Fail	Mine already present
		SET	Temp,1
		STB	Temp,:MineField,Offset
		
		GET	rJ,:rJ
		
		SET	i,2
2H		SUB	i,i,1

		SET	k,2
1H		SUB	k,k,1		
		
		ADD	Temp+1,x,i
		ADD	Temp+2,y,k
		PUSHJ	Temp,:IncMineCount   
		
		BNN	k,1B
		BNN	i,2B
		
		PUT	:rJ,rJ
		SET	$0,1
		POP	1,0
		
Fail		SET	$0,0
		POP	1,0	



		PREFIX :ClearMines:
		
*Local Variables
Offset		IS	$0
Temp		IS	$1

:ClearMines	SET	Temp,0
		SET	Offset,:Columns*:Rows
1H		SUB	Offset,Offset,1

		STB	Temp,:MineField,Offset
		STB	Temp,:MineCounts,Offset
		STB	Temp,:FieldVisible,Offset
		BP	Offset,1B	
		SET	:VisibleCount,0

		POP	0,0	


		PREFIX	:Random;		
		
* Parameters
n		IS $0	Return random number in the range 0 to n-1

*Local Variables
Temp		IS $1

* This multiplier was obtained from Knuth, D.E., "The Art of
* Computer Programming," Vol 2, Seminumerical Algorithms, Third
* Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */

RMUL		GREG	6364136223846793005
RSEED		GREG	1

:Random		BNP	n,ReturnZero

		MULU	RSEED,RSEED,RMUL
		ADDU	RSEED,RSEED,1
		SRU	Temp,RSEED,32
		DIV	Temp,Temp,n
		GET	$0,:rR
		POP	1,0
		

ReturnZero	SET	$0,0
		POP	1,0		

		
* Cange the seed using this function		
		
:RSeed		SET	RSEED,$0
		POP	0,0





		PREFIX	:PlaceMines:
* Parameters
n		IS $0	number of mines to place

* Local Variables
x		IS $1
y		IS $2
rJ		IS $3
max		IS $4
Temp		IS $5


:PlaceMines	BNP	n,Return
		SET	max,:Columns*:Rows
		CMP	Temp,n,max
		CSP	n,Temp,max
		GET	rJ,:rJ

1H		SET	Temp+2,:Columns
		PUSHJ	Temp+1,:Random
		SET	Temp+3,:Rows
		PUSHJ	Temp+2,:Random
		PUSHJ	Temp,:PutMine

		SUB	n,n,Temp
		BP	n,1B

		PUT	:rJ,rJ
Return		POP	0,0



		PREFIX :Display
i		IS	$0
d 		IS	$1
x		IS	$2
y		IS	$3
Temp		IS	$4

GSetPos		IS	#17
GPutStr		IS	#13
GLine		IS	#14
GSetLineColor	IS	#1A
				
Title	BYTE	"    Minesweeper Version 1.0",10,0

:DisplayField	SET	$255,0
		TRAP	0,GSetPos,0
		GETA	$255,Title
		TRAP	0,GPutStr,0
		
		SET	$255,#00FF	Blue
		TRAP	0,GSetLineColor,0
		
		SET	x,:XORIGIN
		SET	d,:WIDTH*:Columns
		SET	i,:Rows
		SET	y,:YORIGIN
1H		SL	$255,x,16
		OR	$255,$255,y
		TRAP	0,GSetPos,0
		ADD	$255,x,d
		SL	$255,$255,16
		OR	$255,$255,y
		TRAP	0,GLine,0
		SUB	i,i,1
		ADD	y,y,:WIDTH
		BNN	i,1B
		
		SET	x,:XORIGIN
		SET	d,:WIDTH*:Rows
		SET	i,:Columns
		SET	y,:YORIGIN
1H		SL	$255,x,16
		OR	$255,$255,y
		TRAP	0,GSetPos,0
		SL	$255,x,16
		ADD	$255,$255,y
		ADD	$255,$255,d
		TRAP	0,GLine,0
		SUB	i,i,1
		ADD	x,x,:WIDTH
		BNN	i,1B
		
		POP	0,0	
			


		PREFIX	:MakeVisible	
* Parameters
x		IS  $0
y		IS  $1
* Local Variables
Offset		IS  $2
OutPtr		IS  $3
Count		IS  $4
rJ		IS  $5
i               IS  $6
k               IS  $7
Temp		IS  $8


* make Field at (x,y) visible
		
:MakeVisible	BN	x,Return	Test Range of Parameters
		BN	y,Return
		CMP	Temp,x,:Columns
		BNN	Temp,Return
		CMP	Temp,y,:Rows
		BNN	Temp,Return

		MUL	Offset,y,:Columns
		ADD	Offset,Offset,x
		LDB	Temp,:FieldVisible,Offset
		BNZ	Temp,Return
		SET	Temp,1
		STB	Temp,:FieldVisible,Offset
		ADD	:VisibleCount,:VisibleCount,1
		
* update Display
GPutChar	IS	#12
		MUL	Temp,x,:WIDTH
		ADD	$255,Temp,:XORIGIN+:WIDTH/4
		SL	$255,$255,16
		MUL	Temp,y,:WIDTH
		ADD	Temp,Temp,:YORIGIN+1		
		OR	$255,$255,Temp
	
		LDB	Temp,:MineField,Offset
		BNZ	Temp,ShowMine
		LDB	Count,:MineCounts,Offset
		
		ADD	Temp,Count,'0'  convert to ASCII digit
		SL	Temp,Temp,32
		OR	$255,$255,Temp
		TRAP	0,GPutChar,0
		BNZ	Count,Return

* The MineCount was zero and we make the neighbours visible.

		GET	rJ,:rJ
		
		SET	i,2
2H		SUB	i,i,1

		SET	k,2
1H		SUB	k,k,1		
		
		ADD	Temp+1,x,i
		ADD	Temp+2,y,k
		PUSHJ	Temp,:MakeVisible  
		
		BNN	k,1B
		BNN	i,2B
		
		PUT	:rJ,rJ
Return		POP	0,0	


ShowMine	SET	Temp,'*'
		SL	Temp,Temp,32
		OR	$255,$255,Temp
		TRAP	0,GPutChar,0
		POP	0,0

		PREFIX :AllVisible:
		
*Local Variables
x		IS	$0
y 		IS	$1
rJ		IS	$2
Temp		IS	$3

:AllVisible	GET	rJ,:rJ
		SET	y,:Rows-1
2H		SET	x,:Columns-1
		
1H		SET	Temp+1,x
		SET	Temp+2,y
		PUSHJ	Temp,:MakeVisible
		
		BZ	:VisibleCount,Return
		SUB	x,x,1
		BNN	x,1B	
		SUB	y,y,1
		BNN	y,2B		

Return		PUT	:rJ,rJ
		POP	0,0	


		PREFIX	:GetValues:

MWaitEvent	IS	#11
:GetValues	TRAP	0,MWaitEvent,0
		SR	$0,$255,32	get the event byte
		AND	$1,$0,#01	left button
		BZ	$1,:GetValues	should be the left button
		AND	$1,$0,#04	down event
		BNZ	$1,:GetValues	should be an up event
		SR	$1,$255,16
		SETL	$2,#FFFF
		AND	$1,$1,$2	x
		SUB	$1,$1,:XORIGIN	- top left
		DIV	$1,$1,:WIDTH	scale
		AND	$0,$255,$2	y
		SUB	$0,$0,:YORIGIN	- top left
		DIV	$0,$0,:WIDTH	scale
		POP	2,0

		PREFIX	:PlayLoop:

*Local Variables
x		IS	$0
y		IS	$1
rJ		IS	$2
Temp		IS	$3

:PlayLoop	GET	rJ,:rJ
		
Loop		SET	Temp,:Rows*:Columns-:Mines
		CMP	Temp,Temp,:VisibleCount
		BNP	Temp,Win
		
		PUSHJ	x,:GetValues
		CMP	Temp,x,:Columns
		BNN	Temp,IllegalValue
		CMP	Temp,y,:Rows
		BNN	Temp,IllegalValue

		SET	Temp+1,x
		SET	Temp+2,y
		PUSHJ	Temp,:MakeVisible
				
		SET	Temp+1,x
		SET	Temp+2,y
		PUSHJ	Temp,:GetMine
		BZ	Temp,Loop

*otherwise we hit a mine and exit
GPutStr		IS	#13
GSetTextColor	IS	#18
GSetPos		IS	#17
		SETML	$255,#00FF	Red
		TRAP	0,GSetTextColor,0
		SET	$255,:YORIGIN+(:Rows+2)*:WIDTH
		TRAP	0,GSetPos,0
		GETA	$255,GameOver
		TRAP	0,GPutStr,0
		PUT	:rJ,rJ
		POP	0,0	
		
GameOver	BYTE	"   GAME OVER !",10,0
				
Win		PUSHJ	Temp,:AllVisible
		SETML	$255,#00FF	Red
		TRAP	0,GSetTextColor,0
		SET	$255,:YORIGIN+(:Rows+2)*:WIDTH
		TRAP	0,GSetPos,0
		GETA	$255,Congratulation
		TRAP	0,GPutStr,0
		PUT	:rJ,rJ
		POP	0,0
			
Congratulation	BYTE	"   CONGRATULATIONS !",10,0

		
IllegalValue	SET	$255,:YORIGIN+(:Rows+2)*:WIDTH
		TRAP	0,GSetPos,0
		GETA	$255,OutOfRange
		TRAP	0,GPutStr,0
		JMP	:PlayLoop

OutOfRange	BYTE	"Value out of Range!",10,0
	
		


		PREFIX	:
		
* Parameters
argc		IS	$0
argv		IS	$1

* Local Variables
i		IS	$2

Temp		IS	$3
		
		
Main		PUSHJ	Temp,ClearMines
		SET	Temp+1,Mines
		PUSHJ	Temp,PlaceMines
		PUSHJ	Temp,DisplayField
		PUSHJ	Temp,PlayLoop		

		TRAP	0,Halt,0

		