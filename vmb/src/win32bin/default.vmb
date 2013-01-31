# default.mmc example configuration file


port 9002
host localhost

#if mother
debug
#on
#exec rom.exe -c "#FILE#"
#exec ram.exe -c "#FILE#"
#exec keyboard.exe -c "#FILE#"
#exec screen.exe -c "#FILE#"
#exec mmixcpu.exe -i hello.mmo 
#terminate
#endif


#if keyboard
address 0x0001000000000000
interrupt 17
#endif

#if screen
address 0x0001000000000008
disableinterrupt 
#interrupt 18
#endif

#if rom
address 0x0000000000000000
file bios.img
#endif

#if ram
size 0x100000
address 0x0000000100000000
#endif

