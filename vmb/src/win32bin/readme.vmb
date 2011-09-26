# readme.vmb configuration file for the readme example


port 9002
host localhost

#if mother
debug
debugmask 0xFFF0
on
exec rom -c "#FILE#"
exec keyboard -c "#FILE#"
exec screen.exe -c "#FILE#"
exec mmixcpu.exe 
#endif

#if rom
address 0x0000000000000000
file readme.img
minimized
#endif

#if keyboard
address 0x0001000000000000
interrupt 17
file readme.txt
#endif

#if screen
address 0x0001000000000008
fontheight 11
disableinterrupt 
#endif

