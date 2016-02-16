	LOC	Data_Segment
	GREG	@
a	OCTA	10
h	BYTE	"Hello World!",10,0

	LOC	#100
Main	LDO	$2,a
	LDA	$255,h
	TRAP	0,Fputs,StdOut
	TRAP	0,Halt,0
