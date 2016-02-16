# default.mmc configuration file for the example


port 9002
host localhost


#if mother
#debug
exec rom
exec ram
exec keyboard
exec screen
exec mmixcpu -i hello.mmo 
#endif


#if keyboard
#debug
address 0x1000000000000
interrupt 17
#endif

#if screen
address 0x1000000000008
interrupt 18
#endif

#if rom
address 0x0000000000000000
file bios.img
#endif

#if ram
size 3145728
address 0x0000000100000000
#endif

