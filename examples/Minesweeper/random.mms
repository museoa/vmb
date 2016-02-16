		PREFIX	:Random:		
		
* Parameters
n		IS $0	Return random number in the range 0 to n-1

*Local Variables
Temp		IS $1

* This multiplier was obtained from Knuth, D.E., "The Art of
* Computer Programming," Vol 2, Seminumerical Algorithms, Third
* Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */

RMUL		GREG	6364136223846793005
RSEED		GREG	0

:Random		BNP	n,ReturnZero

		MULU	RSEED,RSEED,RMUL
		ADDU	RSEED,RSEED,1
		SRU	Temp,RSEED,32
		DIV	Temp,Temp,n
		GET	$0,:rR
		POP	1,0
		

ReturnZero	SET	$0,0
		POP	1,0		

		
* Cange the seed using this function		
		
:RSeed		SET	RSEED,$0
		POP	0,0



* Change the seed using the current time if you want real randomness
* and have a TTimeOfDay Trap and a Timer device

TTimeOfDay	IS	#0F
:RTimeSeed	TRAP	0,TTimeOfDay,0
		SET	RSEED,$255
		POP	0,0

