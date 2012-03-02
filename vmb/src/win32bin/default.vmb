# default.mmc configuration file for the example


port 9002
host localhost
debug

#if mother
debug
#exec rom.exe
exec C:/Programme/vmb/ram.exe
#exec /home/vmb/example3/keyboard.exe
#exec /home/vmb/example3/screen.exe
#exec /home/vmb/example3/mmix.exe -i -B:9002 /home/vmb/example3/hello.mmo 
terminate
#endif


#if keyboard
address 0x1000000000000
interrupt 17
#endif

#if screen
address 0x1000000000008
interrupt 18
#endif

#if rom
address 0x0000000000000000
file /home/vmb/example3/bios.img
#endif

#if ram
size 3145728
address 0x0000000100000000
#endif

