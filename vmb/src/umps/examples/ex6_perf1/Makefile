all: 
	/home/mh/dipl/toolchain/bin/mipsel-linux-as -KPIC coreboot.s -o coreboot
	umps-elf2umps -v -b coreboot
clean:  
	rm -f coreboot
distclean: 
	rm -f coreboot.rom.umps coreboot
