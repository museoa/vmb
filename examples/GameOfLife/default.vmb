# default.mmc configuration file for the example


port 9002
host localhost


#if mother
#debug
exec rom.exe
exec ram.exe
# This is the windows framebuffer device
exec winvram
exec button step
exec button run
exec button stop
exec timer

# exec .\mmix.exe -i -O gol.mmo 
 
#endif


#if rom
address 0x0000000000000000
file bios.img
minimized
#endif

#if ram
size 0x10000000
address 0x0000000100000000
minimized
#endif


#if winvram
#debug
address 0x0002000000000000
fwidth 64
fheight 48
#you can make the actual width/height smaller than the underlying bitmap to get offscscreen memory
width 64
height 48
zoom 10

mouseaddress 0x0001000000000010
interrupt 42

gpuaddress   0x0001000000000020
gpuinterrupt 43

fontheight 15
fontwidth  8

#endif

#if timer
address 0x0003000000000000
interrupt 11

#endif

#if step
label step
color 0xFF0000
interrupt 12
#endif

#if run
label run
color 0x008000
interrupt 13
upinterrupt 14
enable 3
pushbutton
#endif


#if stop
label stop
color 0x0000FF
interrupt 15
#endif



