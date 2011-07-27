# default.mmc configuration file for Linux on the MMIX

port 9002
host localhost
#debug
debugmask 0xFFF0

#if mother
exec rom -c "#FILE#"
exec ram -c "#FILE#"
exec disk -c "#FILE#" 
exec timer -c "#FILE#"
exec screen -c "#FILE#"
exec keyboard -c "#FILE#"
exec button start -c "#FILE#"
exec sevensegment -c "#FILE#"
exec mmixcpu -i
#exec xterm -geometry 40x10 -fn 7x13 -e keyboard -c "#FILE#"
#exec xterm   -fn 7x13 -e screen -c "#FILE#"
#endif

# In the following all devices are listed with their
# default address and interrupt

#if rom
address 0x0000000000000000
file "#PATH#bios.bin"
minimized
#endif

#if ram 
# 256 MB
size    0x10000000
address 0x0000000100000000
minimized
#endif


#if keyboard
address 0x0001000000000000
interrupt 40
#endif

#if screen
address 0x0001000000000008
disableinterrupt
#interrupt 41
#endif

#if winvram
address 0x0002000000000000
# width and height of the bitmap
fwidth 640
fheight 480
# actual width/height can be smaller than the bitmap
# to get offscscreen memory
width 640
height 480
zoom 1
fontheight 15
fontwidth  8

mouseaddress 0x0001000000000010
interrupt 42

gpuaddress   0x0001000000000020
gpuinterrupt 43
#endif

#if sevensegment
address 0x0001000000000080
#endif

#if timer
address 0x0001000000000090
interrupt 44
minimized
#endif

#if led
address 0x00010000000000B0
leds 8
#endif

#if disk
address 0x0003000000000000
interrupt 45
file "#PATH#root.img"
debugmask 0xFFC0
minimized
#endif

# buttons have no address, but interrupts 48 to 55
#if start
color 255
label START
interrupt 48
#endif


