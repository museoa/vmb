# default.mmc configuration file for the example


port 9002
host localhost
debug
verbose

#if mother
on
#exec rom -c #FILE#
#exec keyboard -c #FILE#
#exec screen -c #FILE#
#exec mmixcpu 
#endif

# here we list all the standard devices 
# with their standard address 
# and standard interrupt numbers

#if rom
address 0x0000000000000000
file readme.img
#minimized
#endif

#if ram
size 3145728
address 0x0000000100000000
#minimized
#endif

#if keyboard
address 0x0001000000000000
interrupt 17
#endif

#if screen
address 0x0001000000000008
interrupt 18
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
