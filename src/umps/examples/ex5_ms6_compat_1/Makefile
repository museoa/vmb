# Makefile for mips-linux

DEFS = h/const.h Makefile

CFLAGS = -O0 -ansi -pedantic -Wall -c
LDFLAGS =  -T ../../support/elf32ltsmip.h.umpscore.x 
CC = /home/mh/dipl/toolchain/bin/mipsel-linux-gcc 
LD = /home/mh/dipl/toolchain/bin/mipsel-linux-ld

#main target
all: example.core.umps

example.core.umps: example
	umps-elf2umps -k $<
	ln -sf example.core.umps kernel.core.umps
	ln -sf example.stab.umps kernel.stab.umps

example: example.o module.o
	$(LD) $(LDFLAGS) ../../support/crtso.o example.o module.o ../../support/libumps.o -o $@

module.o: module.c $(DEFS) 
	$(CC) $(CFLAGS) module.c

example.o: example.c $(DEFS) e/module.e
	$(CC) $(CFLAGS) example.c

clean:
	-rm -f *.o example

distclean: clean
	-rm example.*.umps
