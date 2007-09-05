PROG= rom
UTILDIR = ../util
CFLAGS= -g -Wall
CC = gcc


all: $(PROG) bios.rom bios.elf

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) -I $(UTILDIR) $(PROG).c  -L $(UTILDIR) -lvmb  -o $(PROG)



# making the rom bios program cross-compiling on the host with target = mmix

BIN=/usr/local/bin
MAS=$(BIN)/mmix-as
MLD=$(BIN)/mmix-ld
MCC=$(BIN)/mmix-gcc
MCFLAGS= -g -Wall
MLDFLAGS=--entry=_start.
#MLDFLAGS=--section-start .text=0x8000000000000000 


OBJECTS= bios.o io.o

.o: %.mms
	$(MAS) -gdwarf2 --linker-allocated-gregs $^ -o $@

bios.o: bios.s
	$(MCC) $(CFLAGS) -c  $^ -o $@

io.o: io.c
	$(MCC) $(CFLAGS) -mabi=mmixware -mno-base-addresses -O2 -c $^


bios.rom: $(OBJECTS)
	$(MLD) -t -T bios.x $(MLDFLAGS) --oformat binary $(OBJECTS) -o bios.rom

bios.elf: $(OBJECTS)
	$(MLD) -t -T bios.x $(MLDFLAGS) --oformat elf64-mmix $(OBJECTS) -o bios.elf

install: all
	install $(PROG) $(BINDIR)
	install bios.rom $(BINDIR)
	install bios.elf $(BINDIR)

clean:
	rm -f $(PROG) *.o *~ *.rom *.elf





