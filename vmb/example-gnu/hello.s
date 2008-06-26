	LOC   Data_Segment
	GREG  @
hello   BYTE  "Hello world!",10,0

	LOC #100
Main	LDA $255,hello
	TRAP 0,Fputs,StdOut
	TRAP 0,Halt,0
