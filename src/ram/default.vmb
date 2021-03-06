# default.mmc configuration file for  the MMIX hardware


port 9002
host localhost
#host mbox.informatik.fh-muenchen.de
#host lbox.priv.cs.fhm.edu
#debug


#if mother
debug
exec xterm -geometry 40x10 -fn -misc-fixed-*-*-*-*-*-100-*-*-*-*-*-* -e keyboard
exec xterm -geometry 40x10 -fn -misc-fixed-*-*-*-*-*-100-*-*-*-*-*-* -e screen
exec rom
exec ram
#exec xterm -e insight -n --args ./mmix -i -B9002 
exec xterm -e ./mmix -B9002 -g12345
#exec xterm -e ./mmix -B9002 -i
#endif

#if keyboard
address 0x1000000000000
interrupt 17
#endif

#if screen
address 0x1000000000008
interrupt 18
#debug off
#endif

#if rom
address 0x0000000000000000
file bios.rom
#endif

#if ram
size 3145728
address 0x0001000000000000
#endif
