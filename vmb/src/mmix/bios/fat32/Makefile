# making the rom bios program cross-compiling on the host with target = mmix

BIN=/opt/mmix/bin
MAS=$(BIN)/mmix-as
MLD=$(BIN)/mmix-ld
MCC=$(BIN)/mmix-gcc
MOCP=$(BIN)/mmix-objcopy

#MCFLAGS= -ggdb  -O0 -Wall -melf -mabi=mmixware -mno-base-addresses 
MCFLAGS=         -O3 -Wall -melf -mabi=mmixware -mno-base-addresses
#MLDFLAGS=--entry=_start.
MLDFLAGS=  -static \
          --section-start .text=0x8000000000000000 \
          --section-start .bss=0x8000000100000000
# add -t and -M to the loadflags to get more output

MASFLAGS= -gdwarf2 -x 

# some more linker optione
# -lc link in libc
# -static do not link to dynamic libraries
# -nostdlib do not link in standard libraries

OBJECTS= bios.o tables.o dtraps.o ftraps.o terminal.o diskio.o
LIBS=libfat32/libfat32.a
SUBDIRS = libfat32
LFLAGS= -L libfat32 -lfat32  -lc

all:  bios.img bios.elf

pagetab.s: mkpagetab.sh
	mkpagetab.sh > pagetab.s

bios.o: bios.s pagetab.s
	$(MAS) $(MASFLAGS) bios.s -o $@

%.o: %.s
	$(MAS) $(MASFLAGS) $^ -o $@


tables.o: tables.s
	$(MAS) $(MASFLAGS) -no-expand $^ -o $@



%.o: %.c
	$(MCC) $(MCFLAGS) -c $^ -o $@


bios.img: bios.elf
	$(MOCP) -O binary bios.elf bios.img

bios.elf: $(OBJECTS) $(LIBS)
	$(MLD) $(MLDFLAGS) --oformat elf64-mmix $(OBJECTS) $(LFLAGS)  -o $@
	$(MOCP) -O binary bios.elf bios.img



.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@


$(LIBS): $(SUBDIRS)

clean:
	rm -f *.o *~ *.img *.elf
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done





