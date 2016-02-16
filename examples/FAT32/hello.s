	LOC	Data_Segment
	GREG	@

rname	BYTE	"hello.txt",0
orarg    OCTA	rname,BinaryRead
wname	BYTE	"new.txt",0
owarg    OCTA	wname,BinaryWrite

buffer  BYTE	0
bsize	IS	200
	LOC	buffer+bsize
rarg	OCTA	buffer,bsize
warg	OCTA	buffer,0
	

Warg	OCTA	buffer
	OCTA	0

	LOC	#100

Main	LDA	$255,orarg
	TRAP	0,Fopen,5
	LDA	$255,rarg
	TRAP	0,Fread,5
        ADD	$0,$255,bsize
	STO	$0,warg+8
	TRAP	0,Fclose,5

	LDA	$255,buffer
	TRAP	0,Fputs,StdOut

        LDA	$255,owarg
	TRAP	0,Fopen,6
	LDA	$255,warg
	TRAP	0,Fwrite,6
	TRAP	0,Fclose,6

	TRAP	0,Halt,0

1H	GETA	$255,error
	TRAP	0,Fputs,StdOut
	TRAP	0,Halt,0

error	BYTE	"Error reading string!",10,0
	
