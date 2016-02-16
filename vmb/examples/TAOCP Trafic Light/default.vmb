# default.mmc configuration file for Linux on the MMIX

port 9002
host localhost
debugmask 0xFF00

#if mother
#debug
exec rom  -c "#FILE#"
exec ram  -c "#FILE#"
exec timer -c "#FILE#"
exec button -c "#FILE#"
exec mmixcpu  ampel.mmo
exec led delmar -c "#FILE#"
exec led delmarped -c "#FILE#"
exec led berkeley -c "#FILE#"
exec led berkeleyped -c "#FILE#"
#endif


#if rom
address 0x0000000000000000
file bios.img
minimized
#endif

#if ram 
# 256 MB
size    0x10000000
address 0x0000000100000000
minimized
#endif

#if timer
address 0x0001000000000090
interrupt 44
minimized
#endif

#if delmar
address 0x00010000000000B0
leds 3
label DM B
vertical
color2 0xFF0000
picture2 ampel.bmp
color1 0xFFFF00
picture1 ampel.bmp
color0 0x00FF00
picture0 ampel.bmp
#endif

#if delmarped
#debug
address 0x00010000000000B8
leds 2
label Del Mar
color1 0xFF0000
#picture1 ampel.bmp
picture1 halt.bmp
color0 0x00FF00
#picture0 ampel.bmp
picture0 go.bmp
#endif

#if berkeley
address 0x00010000000000C0
leds 3
label B Av
vertical
color2 0xFF0000
picture2 ampel.bmp
color1 0xFFFF00
picture1 ampel.bmp
color0 0x00FF00
picture0 ampel.bmp
#endif

#if berkeleyped
address 0x00010000000000C8
leds 2
label Berkeley
color1 0xFF0000
picture1 dontwalk.bmp
color0 0x00FF00
picture0 walk.bmp
#debug
debugmask 0xFFFF
#endif



#if button
color 0xFF0000
label GO
interrupt 48
#endif


