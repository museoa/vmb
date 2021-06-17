              LOC    #100
%		Defining the programming language
%		Every command consists of two byte, a command and a parameter
%		Parameters can be:
%			Time: 	in 0.1 second increments
%			Lights:	Adding one of green, amber, red, Walk, DontWalk,
%					to one of DelMar or Berkely. See constants below.
%			Sensor:	Zero indicates the Go-Button
%			Count: 	an unsigned repeat-count
              PREFIX :DARE:
CONTINUE      IS     0                              Start program over from the beginning, Parameter=ignored
ON            IS     1                              Switch lights ON, Parameter=Light
OFF           IS     2                              Switch lights OFF, Parameter=Light
WAIT          IS     3                              Wait, Parameter=Time
SENSORCLEAR   IS     4                              Clear the activated status of the sensor, Parameter=Sensor
SENSORWAIT    IS     5                              Wait for the sensor to be activated, Parameter=Sensor
REPEAT        IS     6                              Repeat the following instructions up to END, Parameter=Count
END           IS     7                              End of Repeat Loop. Parameter=ignored

DelMar        IS     0
Walk          IS     8
DontWalk      IS     9
Berkeley      IS     16

green         IS     0
amber         IS     1
red           IS     2

%		The program to control the traffic lights
:PROGRAM      BYTE   ON,DelMar+green
              BYTE   ON,Berkeley+red
              BYTE   ON,DelMar+Walk
              BYTE   ON,Berkeley+DontWalk
              % green on Del Mar

              BYTE   WAIT,180
              BYTE   SENSORWAIT,0

              % start flashing DelMar
              BYTE   OFF,DelMar+Walk
              BYTE   REPEAT,8
              BYTE   ON,DelMar+DontWalk
              BYTE   WAIT,5
              BYTE   OFF,DelMar+DontWalk
              BYTE   WAIT,5
              BYTE   END,0
              BYTE   ON,DelMar+DontWalk
              BYTE   WAIT,40

              % amber on Del Mar
              BYTE   OFF,DelMar+green
              BYTE   ON,DelMar+amber
              BYTE   WAIT,80

              % turn green on Berkeley
              BYTE   OFF,DelMar+amber
              BYTE   ON,DelMar+red
              BYTE   OFF,Berkeley+red
              BYTE   ON,Berkeley+green
              BYTE   OFF,Berkeley+DontWalk
              BYTE   ON,Berkeley+Walk
              % green on Berkeley

              BYTE   WAIT,80

              %start flashing Berkeley
              BYTE   OFF,Berkeley+Walk
              BYTE   REPEAT,8
              BYTE   ON,Berkeley+DontWalk
              BYTE   WAIT,5
              BYTE   OFF,Berkeley+DontWalk
              BYTE   WAIT,5
              BYTE   END,0
              BYTE   ON,Berkeley+DontWalk
              BYTE   WAIT,40

              % amber on Berkeley
              BYTE   SENSORCLEAR,0
              BYTE   OFF,Berkeley+green
              BYTE   ON,Berkeley+amber
              BYTE   WAIT,50

              % prepare to turn DelMar green
              BYTE   OFF,Berkeley+amber
              BYTE   OFF,DelMar+red
              BYTE   OFF,DelMar+DontWalk
              % start over
              BYTE   CONTINUE,0


              PREFIX :
%		Names for TRAPS
LOn           IS     #10
LOff          IS     #11
TWait         IS     #12
BWait         IS     #13
BStatus       IS     #14

tmp           IS     $0

Main          GETA   tmp+1,PROGRAM
              PUSHJ  tmp,Interpreter
              TRAP   0,Halt,0

              PREFIX :Interpreter:
              %Parameter
start         IS     $0
pc            IS     $1
op            IS     $2
arg           IS     $3
rstart        IS     $4
rcount        IS     $5
tmp           IS     $6

:Interpreter  SET    pc,start
Loop          LDBU   op,pc,0
              LDBU   arg,pc,1
              CMP    tmp,op,:DARE:END
              BP     tmp,Illegal
              GETA   tmp,Table
              4ADDU  tmp,op,tmp
              GO     tmp,tmp,0
%		make sure the Table is in the same order
%		as the opcode definitions above
Table         JMP    DOCONTINUE                     IS	0
              JMP    DOON                           IS	1
              JMP    DOOFF                          IS	2
              JMP    DOWAIT                         IS	3
              JMP    DOSENSORCLEAR                  IS	4
              JMP    DOSENSORWAIT                   IS	5
              JMP    DOREPEAT                       IS	6
              JMP    DOEND                          IS	7

DOCONTINUE    SET    pc,start
              JMP    Loop

DOON          SET    $255,1
              SLU    $255,$255,arg
              TRAP   0,:LOn,0
              JMP    Next

DOOFF         SET    $255,1
              SLU    $255,$255,arg
              TRAP   0,:LOff,0
              JMP    Next

DOWAIT        MUL    $255,arg,100                   should be 100, change to make faster or slower
              TRAP   0,:TWait,0
              JMP    Next

DOSENSORCLEAR TRAP   0,:BStatus,0
              JMP    Next

DOSENSORWAIT  TRAP   0,:BWait,0
              JMP    Next

DOREPEAT      ADD    pc,pc,2
              SET    rstart,pc
              SET    rcount,arg
              JMP    Loop

DOEND         SUB    rcount,rcount,1
              BNP    rcount,Next
              SET    pc,rstart
              JMP    Loop

Next          ADD    pc,pc,2
              JMP    Loop

Illegal       GETA   tmp,1F
              SWYM   tmp,5
              SET    pc,start
              JMP    Loop
1H            BYTE   "DEBUG: Illegal Instruction",0

