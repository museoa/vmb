*****************************************
*	Minesweeper                       *
*	Autor: H. Anlauff                 *
*	Oktober 2009                      *
*     Version 0.7                       *
*     Zufallszahlen Generator           *
*     Middlesquare als                  *
*	PUSHJ-Unterprogramm               *
*     Ergebnisse im Speicher            *
*                                       *
*****************************************	

N	IS	10	rows & columns
N2	IS	N*N
M	IS	10	number of mines

	LOC	Data_Segment
	GREG	@

Rand	OCTA	0	Place for random sequence


luecke	IS	$6
seed		IS	luecke+1
count		IS	luecke+2
limit		IS	luecke+3
rbase		IS	luecke+4

		LOC	#100

Main		SET	seed,#1234
		ORML	seed,#abcd
		SET	count,10
		SET	limit,N2
		LDA	rbase,Rand

		PUSHJ	luecke,:Random

		TRAP	0,Halt,0


********************************************************************
*
*		Unterprogramm Random
*		generiert Folge von Zufallszahlen
*		und legt sie im Speicher ab
*	Input	$0	Seed-Wert
*		$1	Anzahl
*		$2	Limit (Wertebereich)
*		$3	Basis-Adresse
*
*********************************************************************
		PREFIX	:Random

*	input parameters

seed		IS	$0
count		IS	$1
limit		IS	$2
base		IS	$3

*	local variables

max		IS	$4	
offset	IS	$5
help		IS	$6
random	IS	$7

:Random	SET	max,#ffff
		ORML	max,#ffff
		SET	offset,0	
		
Loop		MULU	help,seed,seed
		SLU	seed,help,16		next seed
		SRU	seed,seed,32	
		
		MULU	random,seed,limit		scale random number
		DIVU	random,random,max

		STO	random,base,offset	store random number

		SUB	count,count,1
		ADD	offset,offset,8

		BP	count,Loop	


		POP	0,0

		PREFIX	:

********************************************************************
*
*	end of subroutine Random
*
*********************************************************************