# default.mmc configuration file for the example


port 9002
host localhost
debug

#if mother
debug
exec rom.exe bootrom
exec rom.exe userrom

exec C:/Programme/vmb/ram.exe
#exec /home/vmb/example3/mmix.exe -i -B:9002 /home/vmb/example3/hello.mmo 
terminate
#endif

#if bootrom
address 0x0000000000000000
file bios.img
#endif

#if userrom
address 0x0000000000100000
file user.img
#endif




#if ram
size 3145728
address 0x0000000100000000
#endif

