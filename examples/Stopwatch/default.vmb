# default.mmc configuration file for the example


port 9002
host localhost


#if mother
debug
#verbose
exec button.exe start
exec button.exe stop
exec button.exe reset
exec sevensegment.exe
exec timer.exe
exec rom.exe
exec mmix.exe -O
#endif


#if start
color 65280
interrupt 8
#endif

#if stop
color 16711680
interrupt 9
#endif

#if reset
color 0x0
interrupt 10
#endif

#if timer
address 0x0002000000000000
interrupt 11
#endif


#if sevensegment
address 0x0003000000000000
#endif

#if rom
address 0x0000000000000000
file bios.img
#endif

