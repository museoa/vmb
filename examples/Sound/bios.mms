%		Example to demonstrate some features of the VMB Soundcard
%		The code runs entirely in negative addresses. There is no
%		user program. The code can serve as an example on how to write
%		forced TRAP handlers to make sound effects available
%		to user porgrams
%
%		Description
%		The machine boots by setting rT, rTT, rQ, and then
%		uses UNSAVE to set a long list of global registers
%		from a kind of register stack - which is part of the
%		ROM image.Since ROM can not be written, the register stack
%		must not spill to memory.
%		Finaly control is transfered to Main.
%		The main Porgram first preloads a sound file to the soundcard
%		This data can later be played with very little effort.
%		In a loop, the program waits for a button interrupt
%		and plays sound samples:
%		1.	Generating a cosine wave (PCM data) and playing it continuously
%			using double buffering.
%		2.  Playing MP3 data in a continous loop using double buffering.
%		3.	Extracing meta-data (frequency, stereo, ...) from a wav file
%			structure initializing the soundcard appropriately and
%			plaing the sound once from a single large buffer
%		4.	Plaing the preloaded mp3 data.

		PREFIX :
t		IS		$255		temporary variable

%		Boot MMIX
		LOC		#8000000000000000
Boot	GETA	t,:DTrap	set dynamic- and forced-trap  handler
		PUT		:rTT,t
		GETA	t,:FTrap
		PUT		:rT,t
		GETA	$255,TOS	
		UNSAVE	0,$255		initialize rS, rG, and other global registers
		GET		t,:rQ
		PUT		:rQ,0		clear interrupts
		JMP		Main

%		Entry point for a dynamic TRAP	(not used)
:DTrap	PUT		rJ,$255
		NEG		$255,1		enable interrupt $255->rK with resume 1
		RESUME	1

%		Entry point for a forced TRAP (not used)
:FTrap	PUT		rJ,$255
		NEG		$255,1		enable interrupt $255->rK with resume 1
		RESUME	1

%		Defining global registers and values
period	IS	 $254
g254	IS	 4*128		Blocksize for cos example
mask	IS	 $253
g253	IS	 g254-1		Blockmask for cos example
mid		IS	 $252
g252	IS	 g254/2		Blocksize/2
quad	IS	 $251	
g251	IS	g254/4		Blocksize/4
signbit	IS	 $250
g250	IS	 1<<63		Signbit
a0		IS	 $249
g249	IS	 8188.386945373375	Coeficient for cos x = a0 +a2*x^2+a4*x^4
a2		IS	 $248
g248	IS	 -0.6118605667230129
a4		IS	 $247
g247	IS	 6.85730163434783e-6
b0		IS	 $246
g246	IS	 8018.178334504866	Coeficient for cos x = b0 +b2*x^2
b2		IS	 $245
g245	IS	 -0.5077121492401646
SOUND		IS	 $244
g244	IS	 #8003000000000000	Base address for Sound Card
RAM		IS	 $243
g243	IS	 #8000000000300000	Base address for RAM
swap	IS	 $242
g242	IS	 #0102				Convert bigendian WYDE to littleendian
bbit	IS	$241
g241	IS	1<<48				Button interrupt bit
sbit	IS	$240
g240	IS	1<<52				Sound interrupt bit
last	IS	  240

%		The register stack is in ROM! Don't spill the register stack to memory.
%		The following data is loaded during the boot sequence using an UNSAVE instruction. 
		OCTA	0	rL
		OCTA	g240,g241,g242,g243,g244,g245,g246,g247,g248,g249
		OCTA	g250,g251,g252,g253,g254
		OCTA	0	$255
		OCTA	0	rB
		OCTA	0	rD
		OCTA	0	rE
		OCTA	0	rH
		OCTA	0	rJ
		OCTA	0	rM
		OCTA	0	rR
		OCTA	0	rP
		OCTA	0	rW
		OCTA	0	rX
		OCTA	0	rY
		OCTA	0	rZ
TOS		OCTA	last<<56 rG		Top of Stack: UNSAVE loads from here.
		

%		Examples of how to use the sound card (Further details see above).
Main	PUSHJ	t,PreloadMP3
Loop	PUSHJ	t,ButtonWait
		PUSHJ	t,LoopPCM
		PUSHJ	t,ButtonWait
		PUSHJ	t,LoopMP3
		PUSHJ	t,ButtonWait
		PUSHJ	t,OncePCM
		PUSHJ	t,ButtonWait
		PUSHJ	t,OnceMP3p
		PUSHJ	t,ButtonWait
		PUSHJ	t,OnceMP3
		JMP		Loop

:ButtonWait	SYNC	4				power save
			GET		t,:rQ
			BZ		t,:ButtonWait
			PUT		:rQ,0
			POP		0,0

%				Sound card register offsets
Control			IS		#00
DataFormat		IS		#03
Channels		IS		#04
BitsperSample	IS		#05
Samplerate		IS		#06
DMA:address		IS		#10	multiply by DMA number
DMA:size		IS		#08 add to DMA:address

		PREFIX	:LoopPCM:
% 		Geneartin a cosine wave, writing it into two alternating bufffers
% 		and playing them.
addr	IS		$0
size	IS		$1	
n		IS		$2
i		IS		$3
x		IS		$4
buf		IS		$5
bit		IS		$6
rJ		IS		$7
t		IS		$8

Samplerate			IS	22050
SamplesperBuffer	IS	Samplerate*2/2	WYDEs for 1/2 second
Increment			IS	16	take ony every 16th sample (3 octaves higher)

:LoopPCM GET	rJ,:rJ	
		SET		n,50		Maximum number of buffers to play

%		Initialize DMA registers 1 and 2 for double buffering
		SET		addr,:RAM
		ANDNH	addr,#8000					convert to physical address
		STOU	addr,:SOUND,1*:DMA:address	first buffer at RAM
		INCML	addr,2						second buffer at RAM+#20000
		STOU	addr,:SOUND,2*:DMA:address
		SET		size,SamplesperBuffer			
		STOU	size,:SOUND,:DMA:size+1*:DMA:address
		STOU	size,:SOUND,:DMA:size+2*:DMA:address

% 		Fill first buffer using a cos wave
		SET		x,0		number of current sample
		SET		i,SamplesperBuffer
		ADDU	buf,:RAM,i
		NEG		i,i
1H		SET		t+1,x	
		PUSHJ	t,:cos
		MOR		t,t,:swap	WYDE big to little endian
		STWU	t,buf,i
		ADD		i,i,2
		ADD		x,x,Increment
		BN		i,1B

		SETML	t,2
		XOR		buf,buf,t		Switch to next buffer

		SET		i,SamplesperBuffer
		NEG		i,i
1H		SET		t+1,x		
		PUSHJ	t,:cos
		MOR		t,t,:swap	WYDE big to little endian
		STWU	t,buf,i
		ADD		i,i,2
		ADD		x,x,Increment
		BN		i,1B

		SYNC	5	flush cache

%		Start Playing
		SETH	t,#8601		play pcm loop, buffer 1, with interrupt
		ORMH	t,#0201		buffer 2, pcm format
		ORML	t,#0110		1 channel, 16 bit,
		ORL		t,Samplerate
		STOU	t,:SOUND,:Control		execute command


Wait	GET		bit,:rQ			Handle Interrupts
		PUT		:rQ,0
		SUB		n,n,1			This terminates for shure.
		BNP		n,Stop
		AND		t,bit,:sbit
		BNZ		t,Next
		AND		t,bit,:bbit
		BNZ		t,Stop
		SYNC	4				Sleep
		JMP		Wait
	
%		Prepare next buffer
Next	SETML	t,2
		XOR		buf,buf,t
		
		SET		i,SamplesperBuffer
		NEG		i,i
1H		SET		t+1,x
		PUSHJ	t,:cos
		MOR		t,t,:swap		WYDE bigendian to little endian
		STWU	t,buf,i
		ADD		i,i,2
		ADD		x,x,Increment
		BN		i,1B

		SYNC	5				Flush cache
		JMP		Wait

%		Terminating the playing of sound by using the Cancel command.
Stop	PUT		:rQ,0
		SET		t,#7E					Cancel
		STBU	t,:SOUND,:Control		Execute sound cancel
		PUT		:rJ,rJ
		POP		0,0



		PREFIX	:cos:
%		We use this function to compute
%		f(x) = 2^13*Cos[x*2*Pi/period] 
%       using the approximation: 
%		f(x) = 8188.39 -0.00956032 x^2+1.67415*10^-9 x^4
%		in the range 0 to period/4 where period/4 is defined above to be 128

x		IS	 $0	
a		IS	 $1
ai		IS	 $2
y		IS	 $3
t		IS	 $4

:cos	AND		x,x,:mask		using that cos is periodic; now 0<=x<period
		CMP		t,x,:mid		using symetry at period/2	
		BN		t,1F
		SUB		x,:period,x
1H		CMP		t,x,:quad		using symetry at period/4
		BN		t,1F
		SUB		x,:mid,x
1H		ZSNN	t,t,:signbit
		FLOT	x,x
		FMUL	x,x,x	x <- x^2

		FMUL	y,x,:a4
		FADD	y,y,:a2
		FMUL	y,y,x
		FADD	y,y,:a0

%		use this for a degree 2 polynom initializing a0 and a2 with b0 and b2
%		FMUL	y,:b2,x
%		FADD	y,y,:b0

		XOR		y,y,t			invert sign if needed
		FIX		$0,y			round to the next integer
		POP		1,0				return y=f(x)


		PREFIX	:LoopMP3
% 		Play MP3 buffer in a loop.
addr	IS		$0
size	IS		$1	
n		IS		$2
i		IS		$3
k		IS		$4
buf		IS		$5
mp3addr	IS		$6
mp3size	IS		$7
bit		IS		$8
t		IS		$9
SIZE	IS		#4000					size of one sound buffer
	
:LoopMP3 SET		n,30000				Maximum number of buffers to play

%		Initialize DMA buffers 3 and 4 for double buffering
		SET		addr,:RAM
		ANDNH	addr,#8000					convert to physical address
		SET		size,SIZE					
		STOU	addr,:SOUND,3*:DMA:address	first buffer at RAM
		STOU	size,:SOUND,:DMA:size+3*:DMA:address
		INCML	addr,2						second buffer at RAM+#20000
		STOU	addr,:SOUND,4*:DMA:address
		STOU	size,:SOUND,:DMA:size+4*:DMA:address

% 		Use second data file
		GETA	t,:SoundData
		LDOU	mp3addr,t,1*#10		second sound file start
		LDOU	t,t,1*#10+8			second sound file end
		SUBU	mp3size,t,mp3addr

		SET		k,0					Initialize source index.
		SET		i,SIZE				Initialize buffer index.
		ADDU	buf,:RAM,i
		NEG		i,i
1H		LDBU	t,mp3addr,k			Copy.
		STBU	t,buf,i
		ADD		i,i,1
		ADD		k,k,1
		CMP		t,k,mp3size
		ZSN		k,t,k				Reset source index if needed.
		BN		i,1B

		SETML	t,2
		XOR		buf,buf,t			Switch to next buffer.

		SET		i,SIZE				Initialize buffer index.
		NEG		i,i
1H		LDBU	t,mp3addr,k			Copy.
		STBU	t,buf,i
		ADD		i,i,1
		ADD		k,k,1
		CMP		t,k,mp3size
		ZSN		k,t,k				Reset source index if needed.
		BN		i,1B

		SYNC	5					Flush the cache.
		SYNC	4					Sleep shortly.

%		Start Playing
		SETH	t,#8503				Play mp3 loop with interrupt, buffer 3, 
		ORMH	t,#0400				and buffer 4. 
		STOU	t,:SOUND,:Control	Execute the command.

%		Wait for interrupt.
Wait	GET		bit,:rQ				Get interrupts.
		PUT		:rQ,0
		SUB		n,n,1				This terminates sooner or later.
		BNP		n,Stop
		AND		t,bit,:sbit			Sound bit set?
		BNZ		t,Next
		AND		t,bit,:bbit			Button bit set?
		BNZ		t,Stop
		SYNC	4					Sleep again.
		JMP		Wait	
	
%		Prepare the next buffer.
Next	SETML	t,2
		XOR		buf,buf,t			Switch to next buffer.
		
		SET		i,SIZE				Initialize Buffer index.
		NEG		i,i
1H		LDBU	t,mp3addr,k			Copy.
		STBU	t,buf,i
		ADD		i,i,1
		ADD		k,k,1
		CMP		t,k,mp3size
		ZSN		k,t,k				Reset source index if needed.
		BN		i,1B

		SYNC	5	flush cache
		JMP		Wait

%		Terminating the playing of sound by setting the buffer size to zero.
Stop	PUT		:rQ,0
		STCO	0,:SOUND,:DMA:size+3*:DMA:address	size=0
		STCO	0,:SOUND,:DMA:size+4*:DMA:address size=0
		POP		0,0

		
		PREFIX	:OnceMP3p:
		%		PLAY a single mp3 buffer with preloading
addr	IS		$0
size	IS		$1	
t		IS		$2

%		Preload the buffer (done once).
:PreloadMP3		GETA	addr,:SoundData
		LDO		t,addr,0*#10+8			First data file end
		LDOU	addr,addr,0*#10			First data file start
		SUBU	size,t,addr
		ANDNH	addr,#8000				Convert to physical address.
		STOU	addr,:SOUND,7*:DMA:address
		STOU	size,:SOUND,:DMA:size+7*:DMA:address
		SETH	t,#0307					Preload buffer 7.
		STOU	t,:SOUND,:Control		Execute command.
		POP		0,0

%		Play the buffer. (Yes, it's that simple!)
:OnceMP3p	SETH	t,#0107				Play once mp3, buffer 7.
			STOU	t,:SOUND,:Control	Execute command.
			POP		0,0
		

		PREFIX	:OnceMP3:
%		Play two long mp3 buffers without preloading.
addr	IS		$0
size	IS		$1	
t		IS		$2

:OnceMP3 GETA	t,:SoundData
		LDOU	addr,t,1*#10		second sound file start
		LDOU	t,t,1*#10+8			second sound file end
		SUBU	size,t,addr
		ANDNH	addr,#8000			Convert to physical address.
		STOU	addr,:SOUND,8*:DMA:address
		SR		size,size,1			Divide by 2.
		STOU	size,:SOUND,:DMA:size+8*:DMA:address
		ADDU	addr,addr,size
		STOU	addr,:SOUND,9*:DMA:address
		STOU	size,:SOUND,:DMA:size+9*:DMA:address


        SETH	t,#0108				Play once mp3, buffer 8, 
		ORMH	t,#0900				and buffer 9.
		STOU	t,:SOUND,:Control	Execute command.
		POP		0,0

		PREFIX	:OncePCM:
%		PLAY a WAV file			using a single PCM buffer

%		Offsets of information in the wav file header
Format			IS		#14
Channels 		IS		#16
BitsperSample	IS		#22
Samplerate		IS		#18
DataSize		IS		#28
Data			IS		#2C

%		Local variables
wav		IS		$0
value	IS		$1	
t		IS		$2

:OncePCM GETA	wav,:SoundData
		LDOU	wav,wav,2*#10	Address of second Sound file
		LDBU	value,wav,Format
		STBU    value,:SOUND,:DataFormat
		LDBU	value,wav,Channels
		STBU	value,:SOUND,:Channels
		LDBU	value,wav,BitsperSample
		STBU	value,:SOUND,:BitsperSample
		LDW		value,wav,Samplerate
		MOR		value,value,:swap	convert little endian to bigendian
		STW		value,:SOUND,:Samplerate

;		ADDU	wav,wav,#28	Address of data size (little endian)
		LDTU	value,wav,DataSize
		SETML	t,#0102
		ORL		t,#0408
		MOR		value,value,t	convert littleendian to bigendian
;		LDBU	size,wav,3
;		SL		size,size,8
;		LDBU	t,wav,2
;		OR		size,size,t
;		SL		size,size,8
;		LDBU	t,wav,1
;		OR		size,size,t
;		SL		size,size,8
;		LDBU	t,wav,0
;		OR		size,size,t
		STOU	value,:SOUND,:DMA:size+11*:DMA:address		size of buffer 11
		ADDU	value,wav,Data		advance past size
		ANDNH	value,#8000		convert to physical address
		STOU	value,:SOUND,11*:DMA:address		addr of buffer 11
		SET		value,0
		STBU	value,:SOUND,#02	do not use DMA No 2
        SET		value,#020B		play once, pcm buffer 11
		STWU	value,:SOUND,:Control		execute play once pcm buffer 11
		POP		0,0
		

			PREFIX	:
%			Table with start and end addresses of sound data
SoundData	OCTA		mp3AStart	0
			OCTA		mp3AEnd		
			OCTA		mp3BStart	1
			OCTA		mp3BEnd
			OCTA		wavCStart	2
			OCTA		wavCEnd

%			Sound data
			LOC		(@+7)&~7		Alignment
mp3AStart	FILE	"WindowsHardwareRemove.mp3"
mp3AEnd		IS		@

			LOC		(@+7)&~7		Alignment
mp3BStart	FILE	"mond16cbr320+18dB.mp3"
mp3BEnd	IS		@

			LOC		(@+7)&~7		Alignment
wavCStart	FILE	"WindowsNotifySystemGeneric.wav"
wavCEnd		IS		@


		



