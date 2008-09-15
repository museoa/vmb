	LOC	Data_Segment
	GREG	@
name	BYTE	"hello.txt",0
Oarg    OCTA	name,BinaryRead
buffer  BYTE	0
bsize	IS	200
	LOC	buffer+bsize
Rarg	OCTA	buffer
	OCTA	bsize

	LOC	#100
Main	LDA	$255,Oarg
	TRAP	0,Fopen,5
	LDA	$255,Rarg
	TRAP	0,Fread,5
	LDA	$255,buffer
	TRAP	0,Fputs,StdOut
	TRAP	0,Halt,0
