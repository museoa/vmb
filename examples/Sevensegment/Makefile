# making the rom bios program cross-compiling on the host with target = mmix

BIN=.
MAS=$(BIN)\mmix-as
MLD=$(BIN)\mmix-ld
MCC=$(BIN)\mmix-gcc
MCFLAGS= -ggdb -Wall -mabi=mmixware -mno-base-addresses -O2 
#MLDFLAGS=--entry=_start.
MLDFLAGS= -t -nostdlib \
          --section-start .text=0x8000000000000000 \
          --section-start .bss=0x8000000100000000


# some more linker optione
# -lc link in libc
# -static do not link to dynamic libraries
# -nostdlib do not link in standard libraries

OBJECTS= bios.o 

all:  bios.img bios.elf


%.o: %.s
	$(MAS) -gdwarf2 -x $^ -o $@



%.o: %.c
	$(MCC) $(MCFLAGS) -c $^ -o $@


bios.img: $(OBJECTS)
	$(MLD)  $(MLDFLAGS) --oformat binary $(OBJECTS) -o $@


bios.elf: $(OBJECTS)
	$(MLD) $(MLDFLAGS) --oformat elf64-mmix $(OBJECTS) -o $@


clean:
	rm -f *.o *~ *.img *.elf





