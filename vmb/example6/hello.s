	LOC	Data_Segment
	GREG	@
name	BYTE	"hello.txt",0
ORarg    OCTA	name,BinaryRead
OWarg    OCTA	name,BinaryWrite
	
buffer  BYTE	0
bsize	IS	200
	LOC	buffer+bsize
Rarg	OCTA	buffer
	OCTA	bsize

Warg	OCTA	buffer
	OCTA	0

	LOC	#100
Main	LDA	$255,ORarg
	TRAP	0,Fopen,5
	LDA	$255,Rarg
	TRAP	0,Fread,5
	TRAP	0,Fclose,5
	
	LDA	$255,buffer
	TRAP	0,Fputs,StdOut

%	TRAP	0,Halt,0
%      File Output not yet functional

	LDA	$255,Rarg
	TRAP	0,Fgets,StdIn
	BN	$255,1F
	STO	$255,Warg+8
	
	LDA	$255,OWarg
	TRAP	0,Fopen,5
	LDA	$255,Warg
	TRAP	0,Fwrite,5
	TRAP	0,Fclose,5
	
	TRAP	0,Halt,0

1H	GETA	$255,error
	TRAP	0,Fputs,StdOut
	TRAP	0,Halt,0

error	BYTE	"Error reading string!",10,0
	
