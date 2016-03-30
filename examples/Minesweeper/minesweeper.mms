************************************************
* MINESWEEPER
* Autor: M.Ruckert
************************************************

* Constants

Rows		IS 9
Columns 	IS 13
Mines		IS 10


* Traps
MWaitEvent		IS	#11
GPutChar		IS	#12
GPutStr			IS	#13
GLine			IS	#14
GSetPos			IS	#17
GSetTextColor	IS	#18
GSetLineColor	IS	#1A
GBitBltIn		IS	#1B


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
		
FlagsVisible	GREG	@     Boolean field 1 = flag, 0 = no flag
1H		BYTE	0
		LOC	1B+Rows*Columns
		
VisibleCount	GREG	0

XORIGIN		IS	20		x-coordinate of minefield on the screen
YORIGIN		IS	60		y-coordinate of minefield on the screen
SIZE		IS	16		width and height of one field  

BitBltInArg	OCTA	#0010001000000000   WYDE	w=16,h=16,x,y must be OCTA aligned
			OCTA	0							bitmap address



* Basic Subroutines
		LOC	#100

		PREFIX	:ToOffset:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
t		IS  $2

* compute the offset into MineField and the othere arrays 
* based on x and y returns -1 on error.
		
:ToOffset	BN	x,Fail	Test Range of Parameters
		BN	y,Fail
		CMP	t,x,:Columns
		BNN	t,Fail
		CMP	t,y,:Rows
		BNN	t,Fail

		MUL	y,y,:Columns
		ADD	$0,y,x		
		POP	1,0			Return y*Columns+x

Fail	NEG	$0,1
		POP	1,0


		PREFIX	:IncMineCount:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
rJ		IS	$2
offset	IS  $3
t		IS  $4


* increments the Mine Count at (x,y)
		
:IncMineCount	GET		rJ,:rJ
				SET		offset+1,x
				SET		offset+2,y
				PUSHJ	offset,:ToOffset
				BN		offset,1F
				LDB		t,:MineCounts,offset
				ADD		t,t,1
				STB		t,:MineCounts,offset
		
1H				PUT		:rJ,rJ
				POP		0,0


		PREFIX	:GetMine:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
rJ		IS	$2
offset	IS  $3
t		IS  $3


* get the MineField at (x,y)
		
:GetMine	GET		rJ,:rJ
			SET		offset+1,x
			SET		offset+2,y
			PUSHJ	offset,:ToOffset
			BN		offset,1F
			LDB		$0,:MineField,offset
			PUT		:rJ,rJ
			POP		1,0		Return mine.

1H			PUT		:rJ,rJ
			POP		0,0		Return zero.


		PREFIX	:PutMine:
* Parameters
x		IS  $0
y		IS  $1
*Local Variables
rJ		IS  $2
offset	IS  $3
i		IS  $4
k		IS  $5
t		IS  $6


* places a mine in the MineField at (x,y) 
* and updates MineCounts
* returns 1 if successfull, 0 else
		
:PutMine	GET		rJ,:rJ
			SET		offset+1,x
			SET		offset+2,y
			PUSHJ	offset,:ToOffset
			BN		offset,Fail

			LDB	t,:MineField,offset
			BNZ	t,Fail	Mine already present
			SET	t,1
			STB	t,:MineField,offset
		
		SET	i,2
2H		SUB	i,i,1		Loop for i= 1, 0, -1

		SET	k,2
1H		SUB	k,k,1		Loop for k= 1, 0, -1
		
		ADD	t+1,x,i
		ADD	t+2,y,k
		PUSHJ	t,:IncMineCount   
		
		BNN	k,1B
		BNN	i,2B
		
		PUT	:rJ,rJ
		SET	$0,1
		POP	1,0
		
Fail	PUT	:rJ,rJ
		POP	0,0		Return zero.	



			PREFIX :ClearMines:
		
*Local Variables
offset		IS	$0
t		IS	$1

:ClearMines	SET	t,0
			SET	offset,:Columns*:Rows
1H			SUB	offset,offset,1

			STB	t,:MineField,offset
			STB	t,:MineCounts,offset
			STB	t,:FieldVisible,offset
			BP	offset,1B	
			SET	:VisibleCount,0

			POP	0,0	


		PREFIX	:Random:		
		
* Parameters
n			IS 		$0	Return random number in the range 0 to n-1

*Local Variables
t			IS 		$1

* This multiplier was obtained from Knuth, D.E., "The Art of
* Computer Programming," Vol 2, Seminumerical Algorithms, Third
* Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */

RMUL		GREG	6364136223846793005
RSEED		GREG	0

:Random		BNP		n,ReturnZero

			MULU	RSEED,RSEED,RMUL
			ADDU	RSEED,RSEED,1
			SRU		t,RSEED,32
			DIV		t,t,n
			GET		$0,:rR
			POP		1,0
		
ReturnZero	POP		0,0		

		
* Cange the seed using this function		
		
:RSeed		SET	RSEED,$0
			POP	0,0


		PREFIX	:PlaceMines:
*		Place n mines randomly on the field.
* Parameters
n		IS $0	number of mines to place

* Local Variables
x		IS $1
y		IS $2
rJ		IS $3
max		IS $4
t		IS $5


:PlaceMines	BNP	n,Return
		SET		max,:Columns*:Rows/2	Too many mines do not make sense.
		CMP		t,n,max
		CSP		n,t,max
		GET		rJ,:rJ

1H		SET		t+2,:Columns
		PUSHJ	t+1,:Random
		SET		t+3,:Rows
		PUSHJ	t+2,:Random
		PUSHJ	t,:PutMine

		SUB		n,n,t					Substract 1 if successful.
		BP		n,1B

		PUT		:rJ,rJ
Return	POP		0,0



		PREFIX :Display
*		Display the minefield.

* Local Variables
x		IS		$0
y 		IS		$1
rJ		IS		$2
t		IS		$3


				
Title	BYTE	"   Minesweeper Version 1.0",10,0

:DisplayField	GET	rJ,:rJ
		SET		$255,0
		TRAP	0,:GSetPos,0
		GETA	$255,Title
		TRAP	0,:GPutStr,0
		
		SET		x,:Columns
2H		SUB		x,x,1

		SET		y,:Rows
1H		SUB		y,y,1		
		
		SET		t+1,x
		SET		t+2,y
		PUSHJ	t,:DisplayMine
		
		BP		y,1B
		BP		x,2B
		PUT		:rJ,rJ
		POP		0,0	
		
		PREFIX	:DisplayMine:	
* Parameters
x		IS  $0
y		IS  $1
* Local Variables
rJ			IS	$2
offset		IS  $3
bitmap		IS  $4
count		IS  $5
t			IS  $6

:DisplayMine	GET		rJ,:rJ
			SET		offset+1,x
			SET		offset+2,y
			PUSHJ	offset,:ToOffset
			BN		offset,Return

		SET	count,0
		LDB	t,:FieldVisible,offset
		BNZ	t,1F
		
		LDB	t,:FlagsVisible,offset
		BZ	t,2F

		GETA	bitmap,flag
		JMP	show
		
2H		GETA	bitmap,covered
		JMP	show

1H		LDB	t,:MineField,offset
		BZ	t,1F
		GETA	bitmap,mine
		JMP	show

1H		LDB	count,:MineCounts,offset
		GETA	bitmap,empty				
		
show	LDA		$255,:BitBltInArg
		SET		t,16
		STWU	t,$255,0		w
		STWU	t,$255,2		h
		MUL		x,x,:SIZE
		ADD		x,x,:XORIGIN
		STWU	x,$255,4		x
		MUL		y,y,:SIZE
		ADD		y,y,:YORIGIN
		STWU	y,$255,6		y
		MUL		offset,y,:Columns
		STO		bitmap,$255,8	bitmap
		TRAP	0,:GBitBltIn,0
		
		BZ		count,Return
		*		Display mine count
		SETH	$255,#00C0		
		ORMH	$255,#C0C0
		ORML	$255,#0000
		ORL		$255,#00FF
		TRAP	0,:GSetTextColor,0

		ADD		x,x,:SIZE/4
		SL		$255,x,16
		ADD		y,y,1		
		OR		$255,$255,y
		ADD		t,count,'0'  convert to ASCII digit
		SL		t,t,32
		OR		$255,$255,t
		TRAP	0,:GPutChar,0
				
Return	PUT	:rJ,rJ
		POP	0,0
		
*	16x16 bitmaps
*	The data for these bitmaps was produced from the .bmp files using 
*   the "export to C source file" feature of gimp. The C source was then postprocessed
*   using mostly search and replace to obtain mms source code.

covered	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080
	TETRA	#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080
	TETRA	#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00C0C0C0,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00C0C0C0,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080


empty	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0

exploded TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00000000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00000000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00000000,#00FF0000
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00FF0000,#00000000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00808080,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00000000,#00000000,#00FFFFFF,#00FFFFFF,#00000000,#00000000,#00000000,#00000000,#00000000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00000000,#00000000,#00FFFFFF,#00FFFFFF
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00808080,#00FF0000,#00FF0000,#00FF0000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00000000,#00FF0000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00FF0000,#00000000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00000000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00808080,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00000000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00808080,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000

mine	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00000000,#00000000,#00FFFFFF,#00FFFFFF,#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00000000,#00FFFFFF,#00FFFFFF
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0


flag	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00FF0000,#00FF0000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00FF0000
	TETRA	#00FF0000,#00FF0000,#00FF0000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080
	TETRA	#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00FF0000,#00FF0000,#00FF0000,#00FF0000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00FF0000
	TETRA	#00FF0000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00000000,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080
	TETRA	#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00000000,#00000000,#00000000,#00000000
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0
	TETRA	#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0
	TETRA	#00808080,#00808080,#00FFFFFF,#00FFFFFF,#00C0C0C0,#00C0C0C0,#00000000,#00000000,#00000000,#00000000
	TETRA	#00000000,#00000000,#00000000,#00000000,#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00FFFFFF
	TETRA	#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0,#00C0C0C0
	TETRA	#00C0C0C0,#00C0C0C0,#00808080,#00808080,#00FFFFFF,#00C0C0C0,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00C0C0C0,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
	TETRA	#00808080,#00808080,#00808080,#00808080,#00808080,#00808080
		


		PREFIX	:SetFlag
* Parameters
x		IS  $0
y		IS  $1
rJ		IS  $2
* Local Variables
offset		IS  $3
t		IS  $4

:SetFlag	GET		rJ,:rJ
			SET		offset+1,x
			SET		offset+2,y
			PUSHJ	offset,:ToOffset
			BN		offset,Return

			LDB	t,:FlagsVisible,offset
			XOR	t,t,1
			STB	t,:FlagsVisible,offset

			SET	t+1,x
			SET	t+2,y
			PUSHJ	t,:DisplayMine
Return		PUT	:rJ,rJ
			POP	0,0		

		PREFIX	:MakeVisible	
* Parameters
x		IS  $0
y		IS  $1
* Local Variables
rJ		IS  $2
offset		IS  $3
OutPtr		IS  $4
Count		IS  $5

i               IS  $6
k               IS  $7
t		IS  $8


* make Field at (x,y) visible
		
:MakeVisible	GET		rJ,:rJ
			SET		offset+1,x
			SET		offset+2,y
			PUSHJ	offset,:ToOffset
			BN		offset,Return

			LDB	t,:FieldVisible,offset
			BNZ	t,Return
			SET	t,1
			STB	t,:FieldVisible,offset
			ADD	:VisibleCount,:VisibleCount,1
		
			SET	t+1,x
			SET	t+2,y
			PUSHJ	t,:DisplayMine
		
			LDB	t,:MineField,offset
			BNZ	t,Return				Mine hit!
		
			LDB	Count,:MineCounts,offset
		
			BNZ	Count,Return			Mines as neighbours

* The MineCount was zero and we make the neighbours visible.

		
			SET	i,2			Loop for i=1,0,-1
2H			SUB	i,i,1

			SET	k,2			Loop for k=1,0,-1
1H			SUB	k,k,1		
		
			ADD	t+1,x,i
			ADD	t+2,y,k
			PUSHJ	t,:MakeVisible  
		
			BNN	k,1B
			BNN	i,2B
		
Return		PUT	:rJ,rJ
			POP	0,0	

		

		PREFIX :AllVisible:
		
*Local Variables
x		IS	$0
y 		IS	$1
rJ		IS	$2
t		IS	$3

:AllVisible	GET	rJ,:rJ
		SET	y,:Rows-1
2H		SET	x,:Columns-1
		
1H		SET	t+1,x
		SET	t+2,y
		PUSHJ	t,:MakeVisible
		
		BZ	:VisibleCount,Return
		SUB	x,x,1
		BNN	x,1B	
		SUB	y,y,1
		BNN	y,2B		

Return		PUT	:rJ,rJ
		POP	0,0	


		PREFIX	:GetValues:		
* 	return 3 values: 1=left or 0=right button, x, y
*	local variables
x		IS		$0
y		IS		$1
button	IS		$2
t		IS		$3

:GetValues	TRAP	0,:MWaitEvent,0
		SR		button,$255,32	get the event byte
		AND		t,button,#03	left or right button
		BZ		t,:GetValues	should be the left or right button
		AND		t,button,#04	down event
		BNZ		t,:GetValues	should be an up event
		
		AND		button,button,1	main result
		
		SR		x,$255,16
		SETL	t,#FFFF
		AND		x,x,t			x
		SUB		x,x,:XORIGIN	- top left
		DIV		x,x,:SIZE		scale
		AND		y,$255,$3	y
		SUB		y,y,:YORIGIN	- top left
		DIV		y,y,:SIZE		scale
		POP		3,0

		PREFIX	:PlayLoop:

*Local Variables
rJ		IS	$0
flag	IS	$1
x		IS	$2
y		IS	$3
t		IS	$4

:PlayLoop	GET	rJ,:rJ
		
Loop	SET		t,:Rows*:Columns-:Mines
		CMP		t,t,:VisibleCount
		BNP		t,Win				All visible?
		
		PUSHJ	flag,:GetValues    Get flag, x, and y.
		CMP		t,x,:Columns
		BNN		t,IllegalValue
		CMP		t,y,:Rows
		BNN		t,IllegalValue
		
		BNZ		flag,1F
		SET		t+1,x
		SET		t+2,y
		PUSHJ	t,:SetFlag
		JMP		Loop


1H		SET		t+1,x
		SET		t+2,y
		PUSHJ	t,:MakeVisible
				
		SET		t+1,x
		SET		t+2,y
		PUSHJ	t,:GetMine
		BZ		t,Loop

* otherwise we hit a mine and exit
		SETML	$255,#00FF	Red
		TRAP	0,:GSetTextColor,0
		SET		$255,:YORIGIN+(:Rows+2)*:SIZE
		TRAP	0,:GSetPos,0
		GETA	$255,GameOver
		TRAP	0,:GPutStr,0
		PUT		:rJ,rJ
		POP		0,0	
			
		
GameOver	BYTE	"   GAME OVER !",10,0
				
Win		PUSHJ	t,:AllVisible
		SETML	$255,#00FF	Red
		TRAP	0,:GSetTextColor,0
		SET		$255,:YORIGIN+(:Rows+2)*:SIZE
		TRAP	0,:GSetPos,0
		GETA	$255,Congratulation
		TRAP	0,:GPutStr,0
		PUT		:rJ,rJ
		POP		0,0
			
Congratulation	BYTE	"   CONGRATULATIONS !",10,0

		
IllegalValue	SET	$255,:YORIGIN+(:Rows+2)*:SIZE
		TRAP	0,:GSetPos,0
		GETA	$255,OutOfRange
		TRAP	0,:GPutStr,0
		JMP		:PlayLoop

OutOfRange	BYTE	"Value out of Range!",10,0
	
		


		PREFIX	:
		
* Parameters
argc		IS	$0
argv		IS	$1

* Local Variables

t			IS	$2
		
		
Main		PUSHJ	t,ClearMines

%			remove the next 4 lines if you need predicatble random numbers		
TTimeOfDay	IS		#0F
			TRAP	0,:TTimeOfDay,0
			SET		t+1,$255
			PUSHJ	t,:RSeed
		
			SET		t+1,Mines
			PUSHJ	t,PlaceMines
			PUSHJ	t,DisplayField
		
			PUSHJ	t,PlayLoop		

			TRAP	0,:Halt,0

		