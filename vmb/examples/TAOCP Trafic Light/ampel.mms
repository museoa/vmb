% Demo Program shows blinking Trafic lights changing when button is pressed

		LOC	#100

TrapLOn		IS	#10
TrapLOff	IS	#11
TrapTWait	IS	#12
TrapBWait	IS	#13
TrapBStatus	IS	#14


Main		SETML	$255,#0206
		TRAP	0,TrapLOff,0		
		SETL	$255,#0206
		TRAP	0,TrapLOn,0	
1H		SETML	$255,#0101
		TRAP	0,TrapLOn,0
		SET	$255,1000
		TRAP	0,TrapTWait,0
		SETML	$255,#0101
		TRAP	0,TrapLOff,0
		SET	$255,1000
		TRAP	0,TrapTWait,0
		TRAP	0,TrapBStatus,0
		BZ	$255,1B

		
		SET	$255,#0206
		TRAP	0,TrapLOff,0		
		SETML	$255,#0206
		TRAP	0,TrapLOn,0	
1H		SETL	$255,#0101
		TRAP	0,TrapLOn,0
		SET	$255,1000
		TRAP	0,TrapTWait,0
		SETL	$255,#0101
		TRAP	0,TrapLOff,0
		SET	$255,1000
		TRAP	0,TrapTWait,0
		TRAP	0,TrapBStatus,0
		BZ	$255,1B

		JMP	Main