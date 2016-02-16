***********************************************     
*	Minesweeper                             *
*	Autor: H. Anlauff                       *
*	August/Sept 2009                        *
*     Version 0.5                             *
*     interaktive Auswahl                     *
*     der nächsten Zelle                      *
*     Überprüfung auf Mine/Nachbarzellen in   *
*     eigener Subroutine                      *
*     mit Flag für Treffer                    *
*     Iteration mit Abfrage in Main           *
*                                             *
***********************************************	

N	IS	10	rows & columns
M	IS	10	mines

	LOC	Data_Segment
	GREG	@

Ask1		BYTE	10,"Next step? Y/N",10,10,0
Ask2		BYTE	10,"New game? Y/N",10,10,0
GameOver	BYTE	10," G A M E   O V E R !!!",10,0


Answer		OCTA 0   Buffer for the answer
Argin		OCTA Answer,8

		GREG	@

Mines       BYTE  0,0        10 mines
            BYTE  1,1
            BYTE  2,2
            BYTE  3,3     
            BYTE  4,4
            BYTE  5,5
            BYTE  6,6
            BYTE  7,7
            BYTE  8,8
            BYTE  9,9


***         Output field for testing 

Test		   BYTE	"MINESWEEPER TESTSCREEN",10,10
O_Base      GREG	@
O_Field     BYTE	"  A B C D E F G H I J  ",10  
            BYTE  "A                     A",10
            BYTE  "B                     B",10
		BYTE  "C                     C",10
            BYTE  "D                     D",10
		BYTE  "E                     E",10
            BYTE  "F                     F",10
		BYTE  "G                     G",10
            BYTE  "H                     H",10
		BYTE  "I                     I",10
            BYTE  "J                     J",10
		BYTE	"  A B C D E F G H I J  ",10,0

***   Field for playing
***   Zu Beginn sind alle Zellen zugedeckt
***   nach und nach werden sie aufgedeckt,
***   wobei die Nachbarfelder von Minen angezeigt werden

		GREG	@

Play        BYTE  10," M I N E S W E E P E R",10,10 

P_Base      GREG	@
P_Field     BYTE	"  A B C D E F G H I J  ",10  
            BYTE  "A                     A",10
            BYTE  "B                     B",10
		BYTE  "C                     C",10
            BYTE  "D                     D",10
		BYTE  "E                     E",10
            BYTE  "F                     F",10
		BYTE  "G                     G",10
            BYTE  "H                     H",10
		BYTE  "I                     I",10
            BYTE  "J                     J",10
		BYTE	"  A B C D E F G H I J  ",10,0

C_Base	GREG	@

***   Field - Computation
***
***   Meaning: 0            no mine in adjacent cells
***            1 <= i <= 8  i mines in adjacent cells
***            #f           mine in this cell
***            #a		    cell is visible

C_Field	BYTE 0
		LOC	@+(N*2+4)*(N+2)


***   Code-Segment

		LOC	#100

***  Subroutine  Putmine
***  Puts a mine into a cell and actualizes
***  the mine counters of adjacent cells
***  in C_Field and O_Field
***
***  global registers  O_Base, C_Base
***  Input: x, y in $0,$1

		PREFIX	Putmine:
x	IS	$0
y	IS	$1
pos	IS	$2
stern	IS	$3
mine	IS	$4
count IS	$5
test	IS	$6

LL	IS	:N*2+4	Line length	

:Putmine	SET	stern,"*"    mine in O_Field
		SET	mine,#f      mine in C_Field

*** 		Compute position of mine
***		(y+1) * (2*N+4)+ 2*(x+1)
 
		ADD	y,y,1
		SET	pos,2*:N
		ADD	pos,pos,4
		MUL	pos,pos,y
		ADD	x,x,1
		SL	x,x,1
		ADD	pos,pos,x

***		insert mine into fields
		STB	stern,:O_Base,pos
		STB	mine,:C_Base,pos

***		set mine counters of neighbours
***		in C_Field and O_Field

left		SUB	pos,pos,2
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,left_up	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

left_up	SUB	pos,pos,LL		
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,up	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

up		ADD	pos,pos,2
		LDB	count,:C_Base,pos		
		CMP	test,count,#f
		BZ	test,right_up	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

right_up	ADD	pos,pos,2
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,right	mine in this c
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

right		ADD 	pos,pos,LL
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,right_down	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

right_down	ADD	pos,pos,LL
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,down		mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

down		SUB	pos,pos,2
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,left_down	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

left_down	SUB	pos,pos,2
		LDB	count,:C_Base,pos
		CMP	test,count,#f
		BZ	test,Done	mine in this cell
		ADD	count,count,1
		STB	count,:C_Base,pos
		ADD	count,count,48	ASCII
		STB	count,:O_Base,pos

Done		POP	0,0

		PREFIX :

		PREFIX GetNext:

************************************************************
***
***   Subroutine GetNext
***
***   asks the user for next cell to be opened
***
***   Input: none
***
***   Result: Position x,y  and offset pos
***   of cell to be opened
***
***************************************************************


***   Messages to be displayed

	GREG @
Text1	BYTE 10,"select line number y: ",0
Text2	BYTE 10,"select column number x: ",0
Err1	BYTE 10,"wrong value for line/column !!!",0
Err2	BYTE 10,"cell is already visible !!!",0


	GREG	@
InBuf	OCTA	0
InArg OCTA	InBuf,8

***		registers
x	IS	$1
y	IS	$0
pos	IS	$2
test	IS	$3
cell	IS	$4

:GetNext	SWYM

GetNexty	LDA	$255,Text1
		TRAP	0,:Fputs,:StdOut
		LDA	$255,InArg
		TRAP	0,:Fgets,:StdIn

***		read input y
		LDBU	y,InBuf
		SUB	y,y,"A"	ASCII -> int		


***		check input value
		CMP	test,y,0
		BN	test,Error1y
		CMP	test,y,9
		BP	test,Error1y

GetNextx	LDA	$255,Text2
		TRAP	0,:Fputs,:StdOut
		LDA	$255,InArg
		TRAP	0,:Fgets,:StdIn

***		read input x
		LDBU	x,InBuf
		SUB	x,x,"A"	ASCII -> int		

***		check input value
		CMP	test,x,0
		BN	test,Error1x
		CMP	test,x,9
		BP	test,Error1x

*** 		Compute position 
***		(y+1) * (2*N+4)+ 2*(x+1)
 
		ADD	y,y,1
		SET	pos,2*:N
		ADD	pos,pos,4

		MUL	pos,pos,y
		ADD	x,x,1
		SL	x,x,1
		ADD	pos,pos,x

***		check if cell is already visible 

		LDB	cell,:C_Base,pos
		CMP	test,cell,#a		already opened
		BZ	test,Error2

Done		POP	3,0   return x,y and pos

Error1x	LDA	$255,Err1
		TRAP	0,:Fputs,:StdOut
		JMP	GetNextx

Error1y	LDA	$255,Err1
		TRAP	0,:Fputs,:StdOut
		JMP	GetNexty

Error2	LDA	$255,Err2
		TRAP	0,:Fputs,:StdOut
		JMP	GetNexty

		PREFIX :

*************************************************************
***
***   subroutine check cell
***   
***   checks the selected cell and shows the number
***   of adjacent mines
***   game over if a mine is hit
***   
***   input: line/column x/y in $0, $1
***          offset pos in $2
***
***   result:  flag is set if a mine has been hit
***            cell is reveiled in Play_Field
***          
***
**************************************************************

		PREFIX CheckCell:

*** input parameters

x	IS 	$0
y	IS	$1
pos	IS	$2

*** local variables

cell		IS	$3
base		IS	$4
test		IS	$5

***	read selected cell

:CheckCell	LDBU	cell,:C_Base,pos
		CMP	test,cell,#f		mine?
		BZ	test,Bang

		BNZ	cell,Neighbours    	adjacent mines

Zero		SET	cell,176			cell open
		STB	cell,:P_Base,pos
		SET	cell,#a
		STB	cell,:C_Base,pos		cell is visible
		SET	$0,0				clear flag
		POP	1,0

Neighbours	ADD	cell,cell,#30		integer to ASCII
		STB	cell,:P_Base,pos
		SET	cell,#a
		STB	cell,:C_Base,pos		cell is visible
		SET	$0,0				clear flag
		POP	1,0

Bang		SET	$0,1      			set flag for hit
		POP	1,0



	PREFIX :


***  Start of main program

i		IS	$0
m_base	IS	$1
pos		IS	$2
test		IS	$3
x		IS	$4
y		IS	$5
poscopy	IS	$6
cell		IS	$7


Main		SWYM

***      put N mines into the field

New		SET	i,0
		LDA	m_base,Mines

***		read position of next mine

Loop		LDB	x,m_base,i
		ADD	i,i,1
		LDB	y,m_base,i
		ADD	i,i,1

		PUSHJ test,Putmine

***		more mines?

		CMP	test,i,2*N
		BN	test,Loop

***		show the mines in the test field
	
Done		LDA	$255,Test
		TRAP	0,Fputs,StdOut

***		show play field

		LDA	$255,Play
		TRAP	0,Fputs,StdOut

***		next step

Wdhl		PUSHJ test,GetNext

***   	now x,y in x,y, pos in test

		SET	pos,test
		SET	poscopy,test		copy pos

***		check selected cell

         	PUSHJ test,CheckCell

***		mine hit?
		BNZ	test,Over

***		show play field

		LDA	$255,Play
		TRAP	0,Fputs,StdOut

***		ask for next step

*		LDA	$255,Ask1		TRAP	0,Fputs,StdOut
*		LDA	$255,Argin
*		TRAP	0,Fgets,StdIn
*		LDB	test,Answer
*		CMP	test,test,"Y"

*		BZ	test,Wdhl

		JMP	Wdhl			in any case

***		Mine hit: end of game

Over		SET	cell,197            test !!!
		STB	cell,P_Base,pos
		STB	cell,O_Base,pos


***  		Show mines

		SET	i,M-1
		SET	pos,0
NextMine	LDB	cell,O_Base,pos
		CMP	test,cell,"*"
		BNZ	test,Next
		STB	cell,P_Base,pos
		SUB	i,i,1
		BZ	i,Show
Next		ADD	pos,pos,1
		JMP	NextMine
		
***		show play field

Show		LDA	$255,Play
		TRAP	0,Fputs,StdOut

		LDA	$255,GameOver
		TRAP	0,Fputs,StdOut

		TRAP	0,Halt,0



