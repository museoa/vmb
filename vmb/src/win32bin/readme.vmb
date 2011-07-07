# default.mmc configuration file for the readme example


port 9002
host localhost

#if mother
debug
#verbose
on
exec rom -c #FILE#
exec keyboard -c #FILE#
exec screen.exe -c #FILE#
exec mmixcpu.exe 
#endif

#if rom
address 0x0000000000000000
file #PATH#readme.img
minimized
#endif

#if keyboard
address 0x0001000000000000
interrupt 17
file #PATH#readme.txt
#endif

#if screen
address 0x0001000000000008
fontheight 11
disableinterrupt 
#endif

#if winvram
address 0x0002000000000000
fwidth 640
fheight 480
#you can make the actual width/height smaller than the underlying bitmap to get offscscreen memory
width 640
height 480
zoom 1

mouseaddress 0x0001000000000010
interrupt 19

gpuaddress   0x0001000000000020
gpuinterrupt 20

fontheight 15
fontwidth  8

#endif

#if disk
address 0x0002000000000000
#endif
