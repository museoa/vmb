# default.vmb configuration file for testing the sound device


port 9002
host localhost


#if mother
#debug
exec rom
exec ram
exec button
exec sound
#endif

#if rom
address 0x0000000000000000
file bios.img
minimized
#endif

#if ram
size 0x100000
address 0x0000000000300000
minimized
#endif

#if button
interrupt 48
#endif

#if sound
address 0x0003000000000000
interrupt 52
#endif
