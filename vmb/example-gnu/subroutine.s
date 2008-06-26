	LOC	Data_Segment
	GREG	@
a	OCTA	10
b	BYTE	"this is it!"


	LOC	#100

count	IS	$0
text	IS	$1
return 	IS	$2
top	IS	$3
	
sub	GET	return,rJ
	JMP	1F
	
2H	SUB	count,count,1
	SET	top+1,text
	PUSHJ	top,printtext
1H	BNZ	count,2B

	PUT	rJ,return
	SET	$0,0
	POP	1,0
	
Main	LDO	$1,a
	LDA	$2,b
	PUSHJ	$0,sub
	BNZ	$0,Main
	TRAP	0,Halt,0

printtext SET	$255,$0
	  TRAP  0,Fputs,StdOut
	  POP	0,0

	