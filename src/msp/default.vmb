# default.mmc configuration file for the example


port 9002
host localhost

#if mother
debug
debugmask 0xFFF0
on
exec rom.exe -c "#FILE#"
exec ram.exe -c "#FILE#"
exec msp.exe -c "#FILE#"
exec mspio.exe -c "#FILE#"
exec mspmultiplier.exe -c "#FILE#"
exec led.exe -c "#FILE#"
# terminate
#endif

#if rom
address 0x000000000000C000
file led.bin
#endif

#if mspio
address 0x0000000000000020
# output to led address
output 0x0000000000100000
#endif

#if ram
size 512
address 0x0000000000000200
#endif

#if led
size 8
address 0x0000000000100000
#endif

#if mspmultiplier
address 0x0000000000000130
#endif